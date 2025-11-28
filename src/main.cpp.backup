// main.cpp
// ESP32-S3 Wearable Biometric Watch
// Fully fixed & production-ready version
// Updated: November 28, 2025

#include <Arduino.h>
#include "constants.h"
#include "sensor.h"
#include "display.h"
#include "logger.h"

// ------------------- USB Serial (Modern ESP32-S3 Way) -------------------
// HWCDC.h is deprecated. Use built-in Serial (automatically maps to USB CDC)
#define USBSerial Serial  // Just alias it — works on all ESP32-S3 boards

// ------------------- Global Buffers & Results (Single Source!) -------------------
// These are the ONLY definitions — sensor.cpp only has extern declarations
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
int errorCount = 0;

// ------------------- Watchdog (Critical for long buffer fill) -------------------
#include <esp_task_wdt.h>

void enableWatchdog() {
    esp_task_wdt_init(12, true);    // 12-second timeout, panic = reset
    esp_task_wdt_add(NULL);         // Add current task
}

void feedWatchdog() {
    esp_task_wdt_reset();
}

// ------------------- Setup -------------------
void setup() {
    // USB Serial (native on ESP32-S3)
    USBSerial.begin(BAUD_RATE);
    while (!USBSerial && millis() < 3000);  // Wait up to 3s for serial monitor

    USBSerial.println("\n=== ESP32-S3 Biometric Watch Starting ===");
    
    delay(SHORT_DELAY);  // Let power settle

    // Initialize logging first
    if (!logger.init()) {
        USBSerial.println("Logger init failed — continuing without file logging");
    }

    // Enable watchdog early (before long sensor init)
    enableWatchdog();

    // Initialize display first so we can see errors
    display.init();
    display.updateMetrics(0, 0, false, false);  // Show blank

    // Initialize sensor
    if (!sensor.init()) {
        logger.logMessage("FATAL: MAX30102 not found or I2C failed!");
        display.updateMetrics(0, 0, false, false);
        USBSerial.println("Sensor failed — halting");
        while (true) {
            feedWatchdog();
            delay(1000);
        }
    }

    logger.logMessage("System ready. Place finger on sensor.");
    firstRun = true;
    cycleCount = 0;
    errorCount = 0;
}

// ------------------- Cycle Helpers -------------------
unsigned long startCycle() {
    feedWatchdog();
    cycleCount++;
    logger.logMessage("--- Cycle #" + String(cycleCount) + " ---");
    return millis();
}

void showNoSignal() {
    display.updateMetrics(0, 0, false, false);
    delay(UPDATE_DELAY_MS);
}

// ------------------- Main Loop -------------------
void loop() {
    unsigned long cycleStart = startCycle();

    bool success = false;

    if (firstRun) {
        logger.logMessage("Filling initial buffer...");
        success = sensor.fillInitialBuffer();
        firstRun = false;
    } else {
        success = sensor.updateSlidingWindow();
    }

    if (!success) {
        errorCount++;
        logger.logMessage("Sample timeout (error #" + String(errorCount) + ")");
        if (errorCount > 8) {
            logger.logMessage("Too many errors — reinitializing sensor");
            sensor.softReset();
            errorCount = 0;
            firstRun = true;  // Force refull
        }
        showNoSignal();
        return;
    }

    // Good sample — reset error counter
    errorCount = 0;

    sensor.applySmoothingToBuffers();
    
    if (!sensor.checkSignalQuality()) {
        logger.logMessage("Poor signal quality — waiting for finger");
        showNoSignal();
        return;
    }

    sensor.calculateHRSpO2();

    // Log results
    logger.logMessage("Raw IR: " + String(irBuffer[MY_BUFFER_SIZE-1]) + 
                      " | Red: " + String(redBuffer[MY_BUFFER_SIZE-1]));
    
    if (validHeartRate && validSpo2) {
        logger.logMessage("HR: " + String(heartRate) + " bpm | SpO2: " + String(spo2) + "%");
    } else if (validHeartRate) {
        logger.logMessage("HR: " + String(heartRate) + " bpm | SpO2: ---");
    } else {
        logger.logMessage("No valid reading yet");
    }

    // Update display
    display.updateMetrics(heartRate, spo2, validHeartRate, validSpo2);

    // Performance logging
    unsigned long cycleTime = millis() - cycleStart;
    logger.logMessage("Cycle time: " + String(cycleTime) + " ms");

    feedWatchdog();
    delay(UPDATE_DELAY_MS);
}