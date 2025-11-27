// logger.cpp
#include "logger.h"
#include "HWCDC.h"  // For USBSerial

extern HWCDC USBSerial;  // From main.cpp

// Global logger instance
LoggerManager logger;

// Initialize logging: Set up file if enabled
bool LoggerManager::init() {
    if (ENABLE_FILE_LOGGING) {
        if (!LittleFS.begin()) {
            USBSerial.println("Error: LittleFS mount failed - File logging disabled");
            return false;
        }
        logFile = LittleFS.open("/debug.log", "a");  // Append mode
        if (!logFile) {
            USBSerial.println("Error: Failed to open /debug.log - File logging disabled");
            return false;
        }
        USBSerial.println("File logging enabled to /debug.log");
    }
    return true;
}

// Log a message: Output to serial if enabled, append to file if enabled
void LoggerManager::logMessage(const String& msg) {
    if (ENABLE_SERIAL_LOGGING) {
        USBSerial.println(msg);
    }
    if (ENABLE_FILE_LOGGING && logFile) {
        logFile.println(msg);
        logFile.flush();  // Ensure written to flash (note: frequent flushes may wear flash; tune if needed)
    }
}