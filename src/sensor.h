//-----------------------------------
// ESP-32 Wearable Biometric Watch
// sensor.h
// Written 11.27.25 
// Modified by Jason Sharer
//-----------------------------------

// sensor.h
#ifndef SENSOR_H
#define SENSOR_H

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include "constants.h"

// Global Buffers and Results (Will be encapsulated better in a class later if needed)
extern uint32_t irBuffer[MY_BUFFER_SIZE];
extern uint32_t redBuffer[MY_BUFFER_SIZE];
extern int32_t spo2;
extern int32_t heartRate;
extern int8_t validSpo2;
extern int8_t validHeartRate;

// Sensor Manager Class: Handles init, reading, and calculations
class SensorManager {
public:
    bool init();  // Initialize I2C and sensor; returns true on success
    bool fillInitialBuffer();  // Fill buffers first time; returns true if no timeouts
    bool updateSlidingWindow();  // Shift and add new samples; returns true if no timeouts
    bool checkSignalQuality();  // Validate min/max IR; returns true if good
    void calculateHRSpO2();  // Run algorithms to update HR/SpO2 globals

private:
    MAX30105 particleSensor;  // Sensor object (private to encapsulate)
    bool readSampleWithTimeout(uint32_t &red, uint32_t &ir);  // Helper: Read one sample with timeout
};

#endif