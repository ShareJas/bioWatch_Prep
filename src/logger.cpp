// logger.cpp
// ESP32-S3 Biometric Watch – Robust & Safe Logger (2025 Final Version)
// Updated: November 28, 2025

#include "logger.h"
#include <Arduino.h>
#include <LittleFS.h>

// Global instance (used everywhere)
LoggerManager logger;

// ===================================================================
// Public Methods
// ===================================================================

bool LoggerManager::init() {
    if (!ENABLE_SERIAL_LOGGING && !ENABLE_FILE_LOGGING) {
        return true;  // Nothing to do
    }

    if (ENABLE_SERIAL_LOGGING) {
        Serial.begin(BAUD_RATE);
        while (!Serial && millis() < 2000);  // Wait up to 2s
        Serial.println(F("\n=== BioWatch Logger Started ==="));
    }

    if (ENABLE_FILE_LOGGING) {
        if (!LittleFS.begin()) {
            Serial.println(F("LittleFS mount failed – file logging disabled"));
            ENABLE_FILE_LOGGING = false;
            return false;
        }

        // Open in append mode, create if missing
        logFile = LittleFS.open("/biowatch.log", FILE_WRITE);
        if (!logFile) {
            Serial.println(F("Failed to open /biowatch.log"));
            LittleFS.end();
            return false;
        }

        // Seek to end and add clean session separator
        logFile.seek(0, SeekEnd);
        logFile.println(F("\n=== New Session – " __DATE__ " " __TIME__ " ==="));
        logFile.flush();
        Serial.println(F("File logging enabled → /biowatch.log"));
    }

    return true;
}

void LoggerManager::logMessage(const String& msg) {
    String timestamped = "[" + String(millis()) + "] " + msg;

    // Serial output (always fast, no blocking)
    if (ENABLE_SERIAL_LOGGING) {
        Serial.println(timestamped);
    }

    // File output – buffered & safe
    if (ENABLE_FILE_LOGGING && logFile) {
        logFile.println(timestamped);

        // Flush every 15 lines or every 5 seconds (prevents data loss on crash)
        if (++flushCounter >= 15) {
            logFile.flush();
            flushCounter = 0;
        }
    }
}

// Optional: Log raw binary data (for future use)
// void LoggerManager::logRaw(const uint8_t* data, size_t len) { ... }

// ===================================================================
// Destructor – ALWAYS close file properly
// ===================================================================
LoggerManager::~LoggerManager() {
    if (ENABLE_FILE_LOGGING && logFile) {
        logFile.println(F("=== Session End ==="));
        logFile.flush();
        logFile.close();
        LittleFS.end();
        Serial.println(F("Logger shutdown complete"));
    }
}

// ===================================================================
// Private members
// ===================================================================
File LoggerManager::logFile;
uint8_t LoggerManager::flushCounter = 0;