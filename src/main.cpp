// main.cpp
//-----------------------------------
// ESP-32 Wearable Biometric Watch
// main.cpp
// Written 11.27.25 
// Modified by Jason Sharer
//-----------------------------------

// main.cpp
#include <Arduino.h>
#include "constants.h"
#include "sensor.h"
#include "display.h"
#include "logger.h"  // For logging
#include "HWCDC.h"   // For USB serial on ESP32-S3

// Serial object for debugging output via USB
// Inspiration: Standard Arduino serial communication setup, adapted for ESP32-S3's HWCDC (Hardware CDC) to enable USB serial without additional hardware, common in ESP32 projects for easy debugging.
HWCDC USBSerial;

// Module Instances
// These are objects of the SensorManager and DisplayManager classes, which encapsulate sensor and display functionalities respectively.
// Inspiration: Object-oriented design from C++ best practices to promote modularity and reusability, inspired by Arduino library examples like SparkFun's MAX3010x where sensor objects are instantiated.
SensorManager sensor;
DisplayManager display;

// Track first run (static to avoid global)
// This flag ensures the initial buffer fill happens only once at startup.
// Inspiration: Common pattern in signal processing sketches (e.g., from SparkFun heart rate examples) to separate initialization from ongoing updates, preventing redundant full buffer fills.
static bool firstRun = true;

// Cycle counter for logging and potential long-run analysis
// Inspiration: Debugging feature from performance profiling in embedded systems, similar to cycle counters in real-time applications to track iterations and detect issues over time.
unsigned long cycleCount = 0;  // For future logging

int errorCount = 0;  // Global counter for consecutive errors

// Setup function: Runs once at startup to initialize hardware and modules
// Inspiration: Core Arduino framework structure; setup() is the entry point for hardware init, drawn from countless Arduino tutorials and examples.
void setup() {
    // Initialize serial for debugging
    // This sets up USB serial communication at the defined baud rate.
    // Inspiration: Essential for logging in development, based on Arduino Serial.begin() but using HWCDC for ESP32-S3's native USB support.
    USBSerial.begin(BAUD_RATE);
    
    // Wait for serial connection to be established
    // Ensures logs are not missed during early boot.
    // Inspiration: Common safeguard in ESP32 projects to prevent output loss, seen in Espressif's IDF examples.
    while (!USBSerial);  // Wait for connection
    
    // Short delay for hardware stabilization
    // Allows time for power and signals to settle after boot.
    // Inspiration: Recommended in sensor datasheets (e.g., MAX30102) and Arduino init routines to avoid transient errors.
    delay(SHORT_DELAY);  // Stabilize
    
    // Initialize logger (must be after serial begin, as it may use serial for errors)
    // Inspiration: Centralized logging init, from best practices in embedded apps for configurable output.
    if (!logger.init()) {
        USBSerial.println("Warning: Logger init failed - Continuing without file logging");
    }
    
    // Initialize sensor module
    // Calls SensorManager::init() to set up I2C and configure the MAX30102.
    // Inspiration: Modular init from SparkFun library examples, ensuring error handling for failed init.
    if (!sensor.init()) {
        // Log error if init fails
        // Inspiration: Robust error handling from production embedded code to prevent silent failures.
        logger.logMessage("Error: Sensor initialization failed - Halting");
        
        // Halt execution in infinite loop on failure
        // Inspiration: Fail-safe mechanism in safety-critical apps, like wearables, to avoid proceeding with faulty hardware.
        while (true) delay(1000);
    }
    
    // Initialize display module
    // Sets up the ST7789 TFT display and backlight.
    // Inspiration: From Arduino_GFX_Library examples, providing a clean interface for graphics init.
    display.init();
    
    // Log display readiness
    // Inspiration: Debugging milestone logs, common in multi-module setups to confirm boot sequence.
    logger.logMessage("Display ready.");
}

// Starts a new cycle: Increments counter, logs start, and records start time
// Returns the cycle start time for later timing calculations.
// Inspiration: Encapsulating setup/teardown in loops, from structured programming in real-time systems.
unsigned long startCycle() {
    unsigned long cycleStart = millis();  // Start total cycle timer
    cycleCount++;
    logger.logMessage("Cycle #" + String(cycleCount) + " started.");
    return cycleStart;
}

// Processes sensor data: Updates buffers, smooths, checks quality, and calculates metrics
// Returns true if successful (no skips needed).
// Inspiration: Grouping data acquisition/processing steps, similar to pipeline stages in DSP (Digital Signal Processing) apps.
bool processSensorData() {
    bool success = true;
    if (firstRun) {
        success = sensor.fillInitialBuffer();
        firstRun = false;
    } else {
        success = sensor.updateSlidingWindow();
    }
    
    if (!success) {
        errorCount++;
        if (errorCount > 5) {
            sensor.init();  // Re-init
            errorCount = 0;
            sensor.softReset();  // Soft reset sensor
        }
        logger.logMessage("Skipping cycle due to sample error");
        return false;
    }
    
    sensor.applySmoothingToBuffers();  // Smooth raw data before quality check and calc
    
    if (!sensor.checkSignalQuality()) {
        return false;
    }
    
    sensor.calculateHRSpO2();
    return true;
}

// Logs raw PPG and calculated metrics to serial
// Inspiration: Centralized logging function, from debug utilities in embedded firmware to consolidate output handling.
void logRawAndMetrics() {
    // Log latest raw PPG values
    // For debugging signal integrity.
    // Inspiration: Raw data dumping from oscilloscope-like tools in sensor dev, common in Arduino serial plots.
    logger.logMessage("Raw PPG - IR: " + String(irBuffer[MY_BUFFER_SIZE - 1]) + ", Red: " + String(redBuffer[MY_BUFFER_SIZE - 1]));

    // Log calculated metrics if valid
    // Inspiration: Conditional logging from validation patterns in health monitoring apps.
    if (validHeartRate) {
        logger.logMessage("HR: " + String(heartRate) + " bpm");
    } else {
        logger.logMessage("Error: Invalid HR - Noisy data?");
    }
    if (validSpo2) {
        logger.logMessage("SpO2: " + String(spo2) + "%");
    } else {
        logger.logMessage("Error: Invalid SpO2 - Poor signal?");
    }
}

// Logs total cycle time, updates display, and applies delay
// Takes cycleStart for timing.
// Inspiration: Wrapping output and pacing, from UI refresh patterns in event-driven loops.
void updateDisplayAndDelay(unsigned long cycleStart) {
    // Log total cycle time
    // Measures loop performance.
    // Inspiration: Benchmarking from real-time constraints in ESP32 tasks, ensuring cycles stay under thresholds.
    logger.logMessage("Total cycle time: " + String(millis() - cycleStart) + " ms");
    
    // Update display with metrics
    // Refreshes screen with latest values.
    // Inspiration: UI updates from wearable examples, like smartwatch sketches using GFX libraries.
    display.updateMetrics(heartRate, spo2, validHeartRate, validSpo2);
    
    // Delay before next cycle
    // Controls update rate and CPU usage.
    // Inspiration: Pacing in looped systems, from Arduino delay() usages to prevent overload.
    delay(UPDATE_DELAY_MS);
}

// Loop function: Main runtime loop for data acquisition, processing, and output
// Runs repeatedly after setup().
// Inspiration: Arduino's core loop() paradigm, adapted for real-time sensor monitoring like in PPG (Photoplethysmography) applications from health wearable projects (e.g., open-source fitness trackers).
void loop() {
    unsigned long cycleStart = startCycle();
    
    if (!processSensorData()) {
        display.updateMetrics(0, 0, false, false);  // Show "No HR/No SpO2" on skip
        delay(UPDATE_DELAY_MS);  // Skip display/logging on error, but still delay
        return;
    }
    
    logRawAndMetrics();
    updateDisplayAndDelay(cycleStart);
}