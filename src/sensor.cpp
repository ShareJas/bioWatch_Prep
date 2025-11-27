//-----------------------------------
// ESP-32 Wearable Biometric Watch
// sensor.cpp
// Written 11.27.25 
// Modified by Jason Sharer
//-----------------------------------
#include "sensor.h"
#include <cstring>  // For memmove

// Globals
uint32_t    irBuffer[MY_BUFFER_SIZE];
uint32_t    redBuffer[MY_BUFFER_SIZE];
int32_t     spo2 = 0;
int32_t     heartRate = 0;
int8_t      validSpo2 = 0;
int8_t      validHeartRate = 0;

// Initialize sensor and I2C
bool SensorManager::init() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2C_SPEED);
    particleSensor.begin(Wire, I2C_SPEED);
    particleSensor.setup(LED_BRIGHTNESS, SAMPLE_AVERAGE, LED_MODE, SAMPLE_RATE, PULSE_WIDTH, ADC_RANGE);
    return true;
}

// Fill buffer initially
// Fills with what?
// What is SensorManager:: ?
bool SensorManager::fillInitialBuffer() {
    for (int i = 0; i < MY_BUFFER_SIZE; i++) {
        if (!readSampleWithTimeout(redBuffer[i], irBuffer[i])) return false;
    } return true;
}


// Update with sliding window
// How does this work
bool SensorManager::updateSlidingWindow() {
    for (int i = 0; i < SLIDING_ADDITIONS; i++) {
        memmove(redBuffer, redBuffer + 1, (MY_BUFFER_SIZE - 1) * sizeof(uint32_t));
        memmove(irBuffer, irBuffer + 1,   (MY_BUFFER_SIZE - 1) * sizeof(uint32_t));
        if (!readSampleWithTimeout(redBuffer[MY_BUFFER_SIZE - 1], irBuffer[MY_BUFFER_SIZE - 1])) {
            return false; 
        }
    } 
    return true;
}


// Check IR signal quality
// How does this work
bool SensorManager::checkSignalQuality() {
    uint32_t minIR = irBuffer[0], maxIR = irBuffer[0];
    for (int i = 1; i < MY_BUFFER_SIZE; i++) {
        if (irBuffer[i] < minIR) minIR = irBuffer[i];
        if (irBuffer[i] > maxIR) maxIR = irBuffer[i];
    }
    return (minIR >= MIN_IR_THRESHOLD);
}


// Calculate HR and SpO2 using SparkFun algs
void SensorManager::calculateHRSpO2() {
    maxim_heart_rate_and_oxygen_saturation(irBuffer, MY_BUFFER_SIZE, redBuffer, &spo2, &validSpo2, &heartRate, &validHeartRate);
}


// Private helper: Read one sample with timeout
bool SensorManager::readSampleWithTimeout(uint32_t &red, uint32_t &ir) {
    unsigned long start = millis();
    while (!particleSensor.available()) {
        particleSensor.check();
        if (millis() - start > SAMPLE_TIMEOUT_MS) {
            return false;  // Timeout
        }
    }
    red = particleSensor.getRed();
    ir = particleSensor.getIR();
    particleSensor.nextSample();
    return true;
}