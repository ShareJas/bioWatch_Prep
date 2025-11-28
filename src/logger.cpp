// logger.cpp — FINAL WORKING VERSION (no external LittleFS needed)
#include "logger.h"
#include <Arduino.h>

// Use built-in LittleFS from Arduino-ESP32 core
#include <LittleFS.h>          // This is already in the core!

LoggerManager logger;

bool LoggerManager::init() {
    if (ENABLE_FILE_LOGGING) {
        if (!LittleFS.begin()) {
            Serial.println("LittleFS mount failed — file logging disabled");
            return false;
        }
        logFile = LittleFS.open("/debug.log", "a");
        if (logFile) {
            Serial.println("File logging enabled → /debug.log");
            logFile.println("\n=== BioWatch Session Start ===");
            logFile.flush();
        } else {
            Serial.println("Failed to open /debug.log");
            LittleFS.end();
            return false;
        }
    }
    return true;
}

void LoggerManager::logMessage(const String& msg) {
    String line = "[" + String(millis()) + "] " + msg;

    if (ENABLE_SERIAL_LOGGING) {
        Serial.println(line);
    }

    if (ENABLE_FILE_LOGGING && logFile) {
        logFile.println(line);
        static int counter = 0;
        if (++counter >= 20) {
            logFile.flush();
            counter = 0;
        }
    }
}

LoggerManager::~LoggerManager() {
    if (logFile) {
        logFile.close();
        LittleFS.end();
    }
}