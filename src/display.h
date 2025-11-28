// display.h
// ESP32-S3 Biometric Watch – Display Manager (Fixed for small ST7789)
// Updated: November 28, 2025

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "constants.h"


class DisplayManager {
public:
    void init();
    void updateMetrics(int32_t hr, int32_t spo2, bool validHR, bool validSpO2);

    // Public access to gfx – used in main.cpp for temporary messages
    Arduino_GFX *gfx = nullptr;

private:
    Arduino_DataBus *bus = nullptr;
};

#endif // DISPLAY_H