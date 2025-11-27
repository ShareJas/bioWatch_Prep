// main.cpp
#include <Arduino.h>
#include "constants.h"
#include "sensor.h"
#include "display.h"

// Module Instances
SensorManager sensor;
DisplayManager display;

// Track first run (static to avoid global)
static bool firstRun = true;
unsigned long cycleCount = 0;  // For future logging

void setup() {
    delay(SHORT_DELAY);  // Stabilize
    
    if (!sensor.init()) {
        // Error handling: Infinite loop or LED blink; for now, just halt
        while (true) delay(1000);
    }
    
    display.init();
}

void loop() {
    cycleCount++;
    bool success = true;

    if (firstRun) {
        success = sensor.fillInitialBuffer();
        firstRun = false;
    } else success = sensor.updateSlidingWindow();
    
    if (!success) {
        delay(UPDATE_DELAY_MS); return;
    }
    
    if (!sensor.checkSignalQuality()) {
        delay(UPDATE_DELAY_MS); return;
    }
    
    sensor.calculateHRSpO2();
    display.updateMetrics(heartRate, spo2, validHeartRate, validSpo2);
    delay(UPDATE_DELAY_MS);
}