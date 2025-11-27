// sensor.cpp
//-----------------------------------
// ESP-32 Wearable Biometric Watch
// sensor.cpp
// Written 11.27.25 
// Modified by Jason Sharer
//-----------------------------------
// sensor.cpp
#include "sensor.h"
#include <cstring>  // For memmove
#include "logger.h" // For logger.logMessage

// Define globals here
uint32_t irBuffer[MY_BUFFER_SIZE];
uint32_t redBuffer[MY_BUFFER_SIZE];
int32_t spo2 = 0;
int32_t heartRate = 0;
int8_t validSpo2 = 0;
int8_t validHeartRate = 0;

// Initialize sensor and I2C
bool SensorManager::init() {
    logger.logMessage("Debug: Before Wire.begin()");
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2C_SPEED);
    logger.logMessage("Debug: After Wire.begin()");

    logger.logMessage("Debug: Attempting sensor init");
    if (!particleSensor.begin(Wire, I2C_SPEED)) {
        logger.logMessage("Error: Sensor init failed");
        return false;
    }
    logger.logMessage("Sensor initialized!");
    particleSensor.setup(LED_BRIGHTNESS, SAMPLE_AVERAGE, LED_MODE, SAMPLE_RATE, PULSE_WIDTH, ADC_RANGE);
    logger.logMessage("Sensor configured. Place on skin for PPG data.");
    return true;
}

// Buffer validation: Check average IR to detect all-low/zero buffers
bool SensorManager::bufferValidate() {
    uint32_t avgIR = 0;
    for (int i = 0; i < MY_BUFFER_SIZE; i++) avgIR += irBuffer[i];
    avgIR /= MY_BUFFER_SIZE;
    if (avgIR < 5000) {  // Lowered threshold for invalid buffer
        logger.logMessage("Invalid buffer - all low values");
        return false;
    }
    return true;
}

// Fill buffer initially (runs once)
// Note: Fake data commented out; use for testing only
bool SensorManager::fillInitialBuffer() {
    unsigned long sectionStart = millis();
    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        if (!readSampleWithTimeout(redBuffer[i], irBuffer[i])) {
            logger.logMessage("Error: Sample timeout during initial fill");
            return false;
        }
        // Fake data for testing: 
        // irBuffer[i] = 100000 + 5000 * sin(i * 0.1);  // Fake pulse
        // redBuffer[i] = irBuffer[i] * 0.9;
    }   
    logger.logMessage("Initial fill time: " + String(millis() - sectionStart) + " ms");
    return bufferValidate();
}

// Update with sliding window (runs every loop after first)
bool SensorManager::updateSlidingWindow() {
    unsigned long sectionStart = millis();
    for (int i = 0; i < SLIDING_ADDITIONS; i++) {
        // Shift buffers left (discard oldest)
        memmove(redBuffer, redBuffer + 1, (MY_BUFFER_SIZE - 1) * sizeof(uint32_t));
        memmove(irBuffer, irBuffer + 1, (MY_BUFFER_SIZE - 1) * sizeof(uint32_t));

        // Add new sample with timeout
        if (!readSampleWithTimeout(redBuffer[MY_BUFFER_SIZE - 1], irBuffer[MY_BUFFER_SIZE - 1])) {
            logger.logMessage("Error: Sample timeout during sliding add");
            return false;
        }
    }
    logger.logMessage("Sliding add time: " + String(millis() - sectionStart) + " ms");
    return bufferValidate();
}

// Check IR signal quality with added pulsatile check and dynamic LED adjustment
bool SensorManager::checkSignalQuality() {
    unsigned long sectionStart = millis();
    uint32_t minIR = irBuffer[0], maxIR = irBuffer[0];
    for (int i = 1; i < MY_BUFFER_SIZE; i++) {
        if (irBuffer[i] < minIR) minIR = irBuffer[i];
        if (irBuffer[i] > maxIR) maxIR = irBuffer[i];
    }
    if (minIR < MIN_IR_THRESHOLD && LED_BRIGHTNESS < 255) {
        LED_BRIGHTNESS += 50;
        if (LED_BRIGHTNESS > 255) LED_BRIGHTNESS = 255;
        particleSensor.setPulseAmplitudeRed(LED_BRIGHTNESS);
        particleSensor.setPulseAmplitudeIR(LED_BRIGHTNESS);
        logger.logMessage("Increased LED to " + String(LED_BRIGHTNESS));
    }
    logger.logMessage("IR Min/Max: " + String(minIR) + "/" + String(maxIR));

    bool isGood = (minIR >= MIN_IR_THRESHOLD) && (maxIR - minIR >= MIN_PULSATILE_RANGE);
    if (isGood) {
        validCycleCount++;
    } else {
        validCycleCount = 0;
        logger.logMessage("Error: Low signal quality or no pulse - Skipping calc/display");
    }
    logger.logMessage("Signal check time: " + String(millis() - sectionStart) + " ms");
    return (validCycleCount >= CONSECUTIVE_VALID_REQUIRED);
}

// Apply simple moving average to buffers for noise reduction
void SensorManager::applySmoothingToBuffers() {
    uint32_t tempIR[MY_BUFFER_SIZE], tempRed[MY_BUFFER_SIZE];
    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        uint32_t sumIR = 0, sumRed = 0;
        int count = 0;
        for (int j = max(0, i - SMA_WINDOW_SIZE + 1); j <= i; j++) {
            sumIR += irBuffer[j];
            sumRed += redBuffer[j];
            count++;
        }
        tempIR[i] = sumIR / count;
        tempRed[i] = sumRed / count;
    }
    memcpy(irBuffer, tempIR, sizeof(tempIR));
    memcpy(redBuffer, tempRed, sizeof(tempRed));
    logger.logMessage("Buffers smoothed.");
}

// Calculate HR and SpO2, then smooth
void SensorManager::calculateHRSpO2() {
    unsigned long sectionStart = millis();
    maxim_heart_rate_and_oxygen_saturation(irBuffer, MY_BUFFER_SIZE, redBuffer, &spo2, &validSpo2, &heartRate, &validHeartRate);
    logger.logMessage("Calc time: " + String(millis() - sectionStart) + " ms");

    if (validHeartRate) smoothHR();  // Apply post-processing
    else simpleHRCalc();
}

// Post-processing: Reject jumps and average HR
void SensorManager::smoothHR() {
    int prevHR = (hrIndex > 0) ? hrHistory[(hrIndex - 1) % HR_HISTORY_SIZE] : heartRate;
    if (abs(heartRate - prevHR) > MAX_HR_JUMP) {
        logger.logMessage("Rejected HR outlier due to large jump");
        validHeartRate = 0;
        return;
    }

    hrHistory[hrIndex % HR_HISTORY_SIZE] = heartRate;
    hrIndex++;

    int sum = 0, count = min(hrIndex, HR_HISTORY_SIZE);
    for (int i = 0; i < count; i++) {
        sum += hrHistory[i];
    }
    heartRate = sum / count;  // Smoothed average
    logger.logMessage("Smoothed HR: " + String(heartRate));
}

// Simple fallback HR calc if standard alg fails
void SensorManager::simpleHRCalc() {
  int crossings = 0;
  for (int i = 1; i < MY_BUFFER_SIZE; i++) {
    if ((irBuffer[i-1] < irBuffer[i]) && (irBuffer[i] > irBuffer[i+1])) crossings++;  // Peak count approx
  }
  heartRate = crossings * (60 * SAMPLE_RATE / MY_BUFFER_SIZE);  // Rough BPM
  validHeartRate = 1;
}

// Soft reset the sensor (from SparkFun library)
void SensorManager::softReset() {
    particleSensor.softReset();  // Calls library method to reset sensor
    delay(100);  // Short wait for reset to take effect
    init();  // Re-configure after reset
    logger.logMessage("Sensor soft reset performed");
}

// Private helper: Read one sample with timeout
bool SensorManager::readSampleWithTimeout(uint32_t &red, uint32_t &ir) {
    unsigned long start = millis();
    while (!particleSensor.available()) {
        particleSensor.check();
        delay(1);  // Yield CPU
        if (millis() - start > SAMPLE_TIMEOUT_MS) {
            return false;
        }
    }
    red = particleSensor.getRed();
    ir = particleSensor.getIR();
    if (ir < 1000) {
        logger.logMessage("Low IR read: " + String(ir));  // Trace low values
    }
    particleSensor.nextSample();
    return true;
}