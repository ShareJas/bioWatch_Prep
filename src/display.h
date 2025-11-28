// display.h
// ESP32-S3 Biometric Watch – Display Manager
// Updated: November 28, 2025

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino_GFX_Library.h>
#include "constants.h"

class DisplayManager {
public:
    void init();
    void updateMetrics(int32_t hr, int32_t spo2, bool validHR, bool validSpO2);

private:
    Arduino_DataBus *bus = nullptr;
    Arduino_GFX     *gfx = nullptr;

    void drawBigText(int x, int y, const String &text, uint16_t color);
};

#endif