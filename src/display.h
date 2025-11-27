// display.h
//-----------------------------------
// ESP-32 Wearable Biometric Watch
// display.h
// Written 11.27.25 
// Modified by Jason Sharer
//-----------------------------------
// display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino_GFX_Library.h>
#include "constants.h"

// Display Manager Class: Handles init and updates
class DisplayManager {
public:
    void init();  // Initialize display and backlight
    void updateMetrics(int hr, int spo2, bool validHR, bool validSpO2);  // Update text on screen

private:
    Arduino_DataBus *bus;  // SPI bus object
    Arduino_GFX *gfx;      // GFX object for drawing
};

#endif