// sensor.cpp
// ESP32-S3 Wearable Biometric Watch
// MAX30102 Sensor Manager – Fully Fixed & Stable
// Updated: November 28, 2025

#include "sensor.h"
#include "constants.h"
#include "logger.h"

#define SAFE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SAFE_MIN(a, b) ((a) < (b) ? (a) : (b))

// Static member definition (one copy for the whole class)
uint8_t SensorManager::currentLedBrightness = LED_BRIGHTNESS_DEFAULT;

// ===================================================================
// PUBLIC METHODS
// ===================================================================

bool SensorManager::init() {
    logger.logMessage("Initializing MAX30102...");

    Wire.begin(SENSOR_SDA_PIN, SENSOR_SCL_PIN);
    Wire.setClock(SENSOR_I2C_SPEED);

    if (!particleSensor.begin(Wire, SENSOR_I2C_SPEED)) {
        logger.logMessage("ERROR: MAX30102 not found on I2C!");
        return false;
    }

    // Standard high-accuracy settings (used in real medical devices)
    particleSensor.setup(
        LED_BRIGHTNESS_DEFAULT,  // LED current
        SAMPLE_AVERAGE,          // 4
        LED_MODE,                // 2 = Red + IR
        SAMPLE_RATE,             // 100 Hz
        PULSE_WIDTH,             // 411 → 18-bit
        ADC_RANGE                // 4096
    );

    // Apply initial brightness
    particleSensor.setPulseAmplitudeRed(currentLedBrightness);
    particleSensor.setPulseAmplitudeIR(currentLedBrightness);

    validCycleCount = 0;
    hrIndex = 0;
    memset(hrHistory, 0, sizeof(hrHistory));

    logger.logMessage("MAX30102 initialized successfully");
    logger.logMessage("LED brightness set to: " + String(currentLedBrightness));
    return true;
}

void SensorManager::softReset() {
    logger.logMessage("Performing soft reset of MAX30102...");
    particleSensor.softReset();
    delay(150);
    init();  // Re-init everything
    validCycleCount = 0;
    // Remove this line completely:
    // errorCount = 0;
}

// ===================================================================
// BUFFER FILLING
// ===================================================================

bool SensorManager::fillInitialBuffer() {
    logger.logMessage("Filling initial buffer (100 samples)...");
    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        if (!readSampleWithTimeout(redBuffer[i], irBuffer[i])) {
            return false;
        }
        if (i % 25 == 0) {
            logger.logMessage("Filled " + String(i) + "/100 samples");
        }
    }
    logger.logMessage("Initial buffer filled successfully");
    return true;
}

bool SensorManager::updateSlidingWindow() {
    for (int i = 0; i < SLIDING_ADDITIONS; i++) {
        // Shift left (discard oldest)
        memmove(redBuffer, redBuffer + 1, (MY_BUFFER_SIZE - 1) * sizeof(int32_t));
        memmove(irBuffer,  irBuffer + 1,  (MY_BUFFER_SIZE - 1) * sizeof(int32_t));

        // Read newest sample
        if (!readSampleWithTimeout(redBuffer[MY_BUFFER_SIZE-1], irBuffer[MY_BUFFER_SIZE-1])) {
            return false;
        }
    }
    return true;
}

// ===================================================================
// SIGNAL QUALITY & LED AUTO-ADJUST
// ===================================================================
bool SensorManager::checkSignalQuality() {
    int32_t minIR = irBuffer[0], maxIR = irBuffer[0];
    int32_t minRed = redBuffer[0], maxRed = redBuffer[0];
    
    for (int i = 1; i < MY_BUFFER_SIZE; i++) {
        if (irBuffer[i] < minIR) minIR = irBuffer[i];
        if (irBuffer[i] > maxIR) maxIR = irBuffer[i];
        if (redBuffer[i] < minRed) minRed = redBuffer[i];
        if (redBuffer[i] > maxRed) maxRed = redBuffer[i];
    }

    int32_t irPulsatile = maxIR - minIR;
    int32_t redPulsatile = maxRed - minRed;

    // DEBUG (keep for now, remove later)
    logger.logMessage("IR: " + String(minIR) + "-" + String(maxIR) + 
                      " (pulse=" + String(irPulsatile) + ")" +
                      " | Red: " + String(minRed) + "-" + String(maxRed) +
                      " (pulse=" + String(redPulsatile) + ")" +
                      " | LED=" + String(currentLedBrightness));

    // FASTER AUTO-LED — inspired by Maxim's SNR optimization
    if (maxIR > 240000 || maxRed > 240000 || irPulsatile > 200000) {  // Saturation or overdrive
        currentLedBrightness -= 25;
        if (currentLedBrightness < 20) currentLedBrightness = 20;
        particleSensor.setPulseAmplitudeRed(currentLedBrightness);
        particleSensor.setPulseAmplitudeIR(currentLedBrightness);
        logger.logMessage("SATURATION/NOISE → LED ↓ to " + String(currentLedBrightness));
    } else if (irPulsatile < 6000 || redPulsatile < 3000 || minIR < 20000) {  // Weak/no signal
        currentLedBrightness += 30;
        if (currentLedBrightness > 120) currentLedBrightness = 120;
        particleSensor.setPulseAmplitudeRed(currentLedBrightness);
        particleSensor.setPulseAmplitudeIR(currentLedBrightness);
        logger.logMessage("WEAK/NOISE → LED ↑ to " + String(currentLedBrightness));
    }

    bool good = (maxIR < 240000) && (maxRed < 240000) &&
                (irPulsatile >= 6000) && (redPulsatile >= 3000) &&
                (minIR >= 20000);  // Ensure minimum DC level

    validCycleCount = good ? validCycleCount + 1 : 0;
    return (validCycleCount >= 1);
}

// ===================================================================
// SMOOTHING & CALCULATION
// ===================================================================
void SensorManager::bandPassFilterBuffers() {
    // Simple band-pass filter (0.5–4 Hz for PPG, removes DC and high-frequency noise)
    const float alpha = 0.95;  // High-pass alpha (0.5 Hz cutoff at 100 Hz sample)
    const float beta = 0.1;    // Low-pass beta (4 Hz cutoff)

    int32_t hp_ir = irBuffer[0];
    int32_t hp_red = redBuffer[0];
    int32_t lp_ir = irBuffer[0];
    int32_t lp_red = redBuffer[0];

    irBuffer[0] = lp_ir = hp_ir = irBuffer[0];
    redBuffer[0] = lp_red = hp_red = redBuffer[0];

    for (int i = 1; i < MY_BUFFER_SIZE; i++) {
        hp_ir = alpha * hp_ir + alpha * (irBuffer[i] - irBuffer[i-1]);
        lp_ir = beta * hp_ir + (1 - beta) * lp_ir;
        irBuffer[i] = lp_ir;

        hp_red = alpha * hp_red + alpha * (redBuffer[i] - redBuffer[i-1]);
        lp_red = beta * hp_red + (1 - beta) * lp_red;
        redBuffer[i] = lp_red;
    }
}

void SensorManager::applySmoothingToBuffers() {
    int32_t tempIR[MY_BUFFER_SIZE];
    int32_t tempRed[MY_BUFFER_SIZE];

    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        int32_t sumIR = 0, sumRed = 0;
        int start = max(0, i - SMA_WINDOW_SIZE + 1);
        int count = i - start + 1;

        for (int j = start; j <= i; j++) {
            sumIR += irBuffer[j];
            sumRed += redBuffer[j];
        }
        tempIR[i]   = sumIR / count;
        tempRed[i]  = sumRed / count;
    }

    memcpy(irBuffer,  tempIR,   sizeof(tempIR));
    memcpy(redBuffer, tempRed,  sizeof(tempRed));
}

void SensorManager::calculateHRSpO2() {
    // 1. Remove DC component — THIS IS MANDATORY
    long ir_mean = 0, red_mean = 0;
    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        ir_mean  += irBuffer[i];
        red_mean += redBuffer[i];
    }
    ir_mean  /= MY_BUFFER_SIZE;
    red_mean /= MY_BUFFER_SIZE;

    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        irBuffer[i]  -= ir_mean;
        redBuffer[i] -= red_mean;
    }

    // 2. Run algorithm on AC-coupled data
    int32_t n_spo2, n_hr;
    int8_t  valid_spo2, valid_hr;

    maxim_heart_rate_and_oxygen_saturation(
        (uint32_t*)irBuffer, MY_BUFFER_SIZE,
        (uint32_t*)redBuffer,
        &n_spo2, &valid_spo2,
        &n_hr, &valid_hr
    );

    // 3. Restore DC
    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        irBuffer[i]  += ir_mean;
        redBuffer[i] += red_mean;
    }

    // 4. Final results
    validHeartRate = (valid_hr && n_hr >= 40 && n_hr <= 200);
    if (validHeartRate) {
        heartRate = n_hr;
        smoothHR();
    }

    validSpo2 = (valid_spo2 && n_spo2 >= 70 && n_spo2 <= 100);
    spo2 = validSpo2 ? n_spo2 : 0;
}

void SensorManager::smoothHR() {
    // Outlier rejection
    if (hrIndex > 0) {
        int prev = hrHistory[(hrIndex - 1) % HR_HISTORY_SIZE];
        if (abs(heartRate - prev) > MAX_HR_JUMP) {
            logger.logMessage("HR jump rejected: " + String(heartRate) + " → using previous");
            validHeartRate = false;
            return;
        }
    }

    hrHistory[hrIndex % HR_HISTORY_SIZE] = heartRate;
    hrIndex++;

    int count = min(hrIndex, HR_HISTORY_SIZE);
    int32_t sum = 0;
    for (int i = 0; i < count; i++) sum += hrHistory[i];

    heartRate = sum / count;
}

void SensorManager::simpleHRCalc() {
    // Very basic peak counting fallback
    int crossings = 0;
    for (int i = 2; i < MY_BUFFER_SIZE - 1; i++) {
        if (irBuffer[i-1] < irBuffer[i] && irBuffer[i] > irBuffer[i+1]) {
            crossings++;
        }
    }
    heartRate = crossings * (60 * SAMPLE_RATE / MY_BUFFER_SIZE);
    validHeartRate = 1;
    logger.logMessage("Fallback HR: " + String(heartRate) + " bpm");
}

// ===================================================================
// PRIVATE HELPER
// ===================================================================

bool SensorManager::readSampleWithTimeout(int32_t &red, int32_t &ir) {
    unsigned long start = millis();

    while (!particleSensor.available()) {
        particleSensor.check();
        if (millis() - start > SAMPLE_TIMEOUT_MS) {
            logger.logMessage("Sample timeout!");
            return false;
        }
        yield();
    }

    red = particleSensor.getRed();
    ir  = particleSensor.getIR();
    particleSensor.nextSample();

    return true;
}