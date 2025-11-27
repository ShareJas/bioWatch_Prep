// logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "constants.h"
#include <LittleFS.h>  // For file system operations

// Logger Manager Class: Handles logging to serial and/or file (toggleable via constants.h)
class LoggerManager {
public:
    bool init();  // Initialize file logging if enabled; returns true on success
    void logMessage(const String& msg);  // Log a message to serial and/or file based on enables

private:
    File logFile;  // File handle for /debug.log (appended if exists)
};

extern LoggerManager logger;  // Global instance for easy access

#endif