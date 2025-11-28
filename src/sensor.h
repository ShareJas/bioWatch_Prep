// sensor.h
// ESP32-S3 Wearable Biometric Watch
// MAX30102 Sensor Manager – Clean & Fixed Version
// Updated: November 28, 2025

#ifndef SENSOR_H
#define SENSOR_H

#include <Wire.h>
#include "MAX30105.h"           // SparkFun MAX3010x library
#include "spo2_algorithm.h"     // Maxim's official SpO2 algorithm
#include "heartRate.h"          // Maxim's heart rate helpers
#include "constants.h"
#include "logger.h"

// ===================================================================
// GLOBAL BUFFERS & RESULTS (declared extern — defined ONLY in main.cpp)
// ===================================================================
// Critical: These must be int32_t — Maxim's algorithm expects signed 32-bit!
extern int32_t irBuffer[MY_BUFFER_SIZE];
extern int32_t redBuffer[MY_BUFFER_SIZE];

extern int32_t spo2;          // Calculated SpO2 value (%)
extern int32_t heartRate;     // Calculated heart rate (bpm)
extern bool    validSpo2;     // True if SpO2 is valid
extern bool    validHeartRate;// True if HR is valid

// ===================================================================
// SensorManager Class
// ===================================================================
class SensorManager {
public:
    bool init();                          // Initialize I2C + MAX30102
    bool fillInitialBuffer();             // Fill buffer on first run
    bool updateSlidingWindow();           // Shift + add new samples
    bool checkSignalQuality();            // Finger detection + pulsatile check
    void bandPassFilterBuffers();         // Band Pass Filter
    void applySmoothingToBuffers();       // Simple moving average filter
    void calculateHRSpO2();               // Run Maxim algorithms + post-process
    void softReset();                     // Full sensor reset + re-init

private:
    MAX30105 particleSensor;              // The actual sensor object

    // LED brightness control (auto-adjusts based on signal strength)
    static uint8_t currentLedBrightness;

    // Signal quality tracking
    int validCycleCount = 0;

    // HR smoothing
    int hrHistory[HR_HISTORY_SIZE] = {0};
    int hrIndex = 0;

    // Private helpers
    bool readSampleWithTimeout(int32_t &red, int32_t &ir);
    void adjustLedBrightness();
    void smoothHR();
    void simpleHRCalc();                  // Fallback if Maxim algo fails
};

#endif // SENSOR_H