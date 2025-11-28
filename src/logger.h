// logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "constants.h"
#include <LittleFS.h>
#include <FS.h>

class LoggerManager {
public:
    bool init();
    void logMessage(const String& msg);
    ~LoggerManager();          // ← ADD THIS LINE

private:
    File logFile;
};

extern LoggerManager logger;

#endif