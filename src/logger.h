/// logger.h
// ESP32-S3 Biometric Watch – Logger Header (Final 2025 Version)
// Updated: November 28, 2025

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include "constants.h"   // For ENABLE_SERIAL_LOGGING / ENABLE_FILE_LOGGING flags

class LoggerManager {
public:
    bool init();                    // Initialize serial + optional file logging
    void logMessage(const String& msg);

    ~LoggerManager();               // Ensures file is closed cleanly on reboot/shutdown

private:
    static File logFile;            // The open log file handle
    static uint8_t flushCounter;    // Counts lines between flushes
};

extern LoggerManager logger;        // Global instance used everywhere

#endif // LOGGER_H