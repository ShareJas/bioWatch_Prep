//-----------------------------------
// ESP-32 Wearable Biometric Watch
// display.cpp
// Written 11.27.25 
// Modified by Jason Sharer
//-----------------------------------
#include "display.h"

// Colors (from Arduino_GFX)
#define BLACK 0x0000
#define RED   0xF800

void DisplayManager::init() {
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
    gfx = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);
    gfx->begin();
    gfx->fillScreen(BLACK);
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);  // Backlight on
}

void DisplayManager::updateMetrics(int hr, int spo2, bool validHR, bool validSpO2) {
    gfx->fillRect(10, 10, 200, 60, BLACK);
    gfx->setCursor(10, 10);
    gfx->setTextColor(RED);
    gfx->setTextSize(2);
    gfx->println(validHR ? "HR: " + String(hr) : "No HR");
    gfx->setCursor(10, 40);
    gfx->println(validSpO2 ? "SpO2: " + String(spo2) : "No SpO2");
}