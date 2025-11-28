// main.cpp
// ESP32-S3 Wearable Biometric Watch – FINAL STABLE VERSION
// Updated: November 28, 2025

#include <Arduino.h>
#include "constants.h"
#include "sensor.h"
#include "display.h"
#include "logger.h"

// ------------------- USB Serial (Modern ESP32-S3 Way) -------------------
#define USBSerial Serial  // Native USB CDC – no HWCDC needed

// ------------------- Global Buffers & Results -------------------
int32_t irBuffer[MY_BUFFER_SIZE];
int32_t redBuffer[MY_BUFFER_SIZE];

int32_t spo2 = 0;
int32_t heartRate = 0;
bool validSpo2 = false;
bool validHeartRate = false;

// ------------------- Module Instances -------------------
SensorManager sensor;
DisplayManager display;

// ------------------- Runtime State -------------------
static bool firstRun = true;
unsigned long cycleCount = 0;
static uint8_t consecutiveGoodCycles = 0;

// ------------------- Watchdog -------------------
#include <esp_task_wdt.h>

void enableWatchdog() {
    esp_task_wdt_init(12, true);    // 12-second timeout → plenty for 50-sample fill
    esp_task_wdt_add(NULL);
}

void feedWatchdog() {
    esp_task_wdt_reset();
}

// ------------------- Setup -------------------
void setup() {
    USBSerial.begin(BAUD_RATE);
    while (!USBSerial && millis() < 3000);  // Wait for serial monitor

    USBSerial.println(F("\n=== ESP32-S3 BioWatch Starting ==="));
    delay(SHORT_DELAY);

    logger.init();                    // Logging first (optional file logging)
    enableWatchdog();

    display.init();
    display.updateMetrics(0, 0, false, false);

    // --- Sensor Init (with retry logic) ---
    int initAttempts = 0;
    while (!sensor.init() && initAttempts < 3) {
        logger.logMessage("MAX30102 init failed – retry " + String(++initAttempts));
        delay(500);
        feedWatchdog();
    }
    if (initAttempts >= 3) {
        logger.logMessage("FATAL: MAX30102 not responding – halting");
        display.updateMetrics(0, 0, false, false);
        while (true) { feedWatchdog(); delay(1000); }
    }

    logger.logMessage("System ready – place finger on sensor");
    firstRun = true;
    cycleCount = 0;
    consecutiveGoodCycles = 0;
}

// ------------------- Main Loop -------------------
void loop() {
    feedWatchdog();
    unsigned long cycleStart = millis();
    cycleCount++;

    if (cycleCount % 10 == 1) {  // Log only every ~2 seconds to reduce spam
        logger.logMessage("--- Cycle #" + String(cycleCount) + " ---");
    }

    bool sampleSuccess = false;

    // =========================================================
    // 1. Fill initial buffer (only on first run or after reset)
    // =========================================================
    if (firstRun) {
        logger.logMessage("Filling initial buffer (50 samples @ ~100Hz)...");
        display.updateMetrics(0, 0, false, false);
        gfx->setTextSize(2); gfx->setTextColor(0xFFFF);
        gfx->setCursor(20, 120); gfx->print("Reading sensor...");

        sampleSuccess = sensor.fillInitialBuffer();

        if (!sampleSuccess) {
            logger.logMessage("Initial fill timeout – restarting sensor");
            sensor.softReset();
            firstRun = true;           // Try again next loop
            delay(500);
            return;
        }

        // After filling, immediately check signal quality
        if (!sensor.checkSignalQuality()) {
            logger.logMessage("No finger detected after fill – waiting...");
            display.updateMetrics(0, 0, false, false);
            firstRun = false;          // Allow sliding window next cycle
            consecutiveGoodCycles = 0;
            delay(UPDATE_DELAY_MS);
            return;
        }

        firstRun = false;
        logger.logMessage("Initial buffer filled & finger detected!");
    }
    // =========================================================
    // 2. Normal operation – sliding window update
    // =========================================================
    else {
        sampleSuccess = sensor.updateSlidingWindow();
        if (!sampleSuccess) {
            logger.logMessage("Sample timeout during sliding window");
            consecutiveGoodCycles = 0;
            showNoFingerMessage();
            delay(UPDATE_DELAY_MS);
            return;
        }
    }

    // =========================================================
    // 3. Pre-processing
    // =========================================================
    sensor.applySmoothingToBuffers();
    sensor.bandPassFilterBuffers();        // ← Re-enabled: essential for clean signal!

    // =========================================================
    // 4. Signal Quality Check (finger detection)
    // =========================================================
    if (!sensor.checkSignalQuality()) {
        consecutiveGoodCycles = 0;
        showNoFingerMessage();
        delay(UPDATE_DELAY_MS);
        return;
    }

    // Finger is on → require several consecutive good cycles before trusting
    consecutiveGoodCycles++;
    if (consecutiveGoodCycles < CONSECUTIVE_VALID_REQUIRED) {
        logger.logMessage("Finger detected – stabilizing... (" + String(consecutiveGoodCycles) + "/" + String(CONSECUTIVE_VALID_REQUIRED) + ")");
        display.updateMetrics(0, 0, false, false);
        delay(UPDATE_DELAY_MS);
        return;
    }

    // =========================================================
    // 5. Calculate HR & SpO2
    // =========================================================
    sensor.calculateHRSpO2();

    // =========================================================
    // 6. Display & Log Results
    // =========================================================
    display.updateMetrics(heartRate, spo2, validHeartRate, validSpo2);

    if (validHeartRate && validSpo2) {
        logger.logMessage("HR: " + String(heartRate) + " bpm | SpO2: " + String(spo2) + "%");
    } else if (validHeartRate) {
        logger.logMessage("HR: " + String(heartRate) + " bpm | SpO2: ---");
    } else {
        logger.logMessage("Calculating...");
    }

    // Optional: log cycle time every 10 cycles
    if (cycleCount % 10 == 0) {
        unsigned long cycleTime = millis() - cycleStart;
        logger.logMessage("Cycle time: " + String(cycleTime) + " ms");
    }

    delay(UPDATE_DELAY_MS);
}

// =========================================================
// Helper: Show "no finger" screen
// =========================================================
void showNoFingerMessage() {
    display.updateMetrics(0, 0, false, false);
    gfx->fillScreen(0x0000);
    gfx->setTextSize(2);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(30, 100);
    gfx->print("Place finger");
    gfx->setCursor(50, 140);
    gfx->print("on sensor");
}