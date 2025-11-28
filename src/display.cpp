// display.cpp – FIXED & CENTERED FOR ALL COMMON SMALL ST7789 SIZES
// Updated: November 28, 2025

#include "display.h"
#include <Arduino.h>

// ================================================================
// CHOOSE ONLY ONE OF THESE THREE BLOCKS (comment out the others)
// ================================================================

// ───── OPTION 1: 135×240 (most common LilyGO T-Display-S3, TTGO T-Watch, etc.)
#define TFT_WIDTH   135
#define TFT_HEIGHT  240
#define ROTATION    1   // 1 or 3 usually works

// ───── OPTION 2: 240×240 square (many cheap round-corner watches)
// #define TFT_WIDTH   240
// #define TFT_HEIGHT  240
// #define ROTATION    0

// ───── OPTION 3: 128×128 round display (rare, but exists)
// #define TFT_WIDTH   128
// #define TFT_HEIGHT  128
// #define ROTATION    0

// ================================================================

// Colors
#define COLOR_BG      0x0000
#define COLOR_TEXT    0xFFFF
#define COLOR_HR      0xF800   // Red
#define COLOR_SPO2    0x001F   // Blue
#define COLOR_WARN    0xFD20   // Orange
#define COLOR_GOOD    0x07E0   // Green
#define COLOR_DIM     0x7BEF   // Light gray

void DisplayManager::init() {
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI, -1);
    gfx = new Arduino_ST7789(bus, LCD_RST, ROTATION, true, TFT_WIDTH, TFT_HEIGHT);

    gfx->begin();
    gfx->fillScreen(COLOR_BG);
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    // Startup splash
    gfx->setTextColor(COLOR_GOOD);
    gfx->setTextSize(2);
    gfx->setCursor((TFT_WIDTH - 84) / 2, TFT_HEIGHT / 2 - 40);  // 84 = 7 chars × 12px
    gfx->print("BIOWATCH");

    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_DIM);
    gfx->setCursor((TFT_WIDTH - 156) / 2, TFT_HEIGHT / 2 + 10);
    gfx->print("Place finger on sensor");

    delay(1800);
    gfx->fillScreen(COLOR_BG);
}

void DisplayManager::updateMetrics(int32_t hr, int32_t spo2, bool validHR, bool validSpO2) {
    gfx->fillScreen(COLOR_BG);

    // ───── Heart Rate ─────
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(10, 15);
    gfx->print("Heart Rate");

    if (validHR && hr >= 40 && hr <= 200) {
        gfx->setTextSize(5);
        gfx->setTextColor(COLOR_HR);
        char buf[5];
        sprintf(buf, "%d", hr);
        int16_t x1, y1; uint16_t w, h;
        gfx->getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
        gfx->setCursor((TFT_WIDTH - w) / 2, 55);
        gfx->print(buf);

        gfx->setTextSize(2);
        gfx->setTextColor(COLOR_HR);
        gfx->setCursor(TFT_WIDTH - 58, 80);
        gfx->print("BPM");
    } else {
        gfx->setTextSize(4);
        gfx->setTextColor(COLOR_WARN);
        gfx->setCursor((TFT_WIDTH - 60) / 2, 70);
        gfx->print("--");
    }

    // ───── SpO2 ─────
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(10, 120);
    gfx->print("SpO2");

    if (validSpO2 && spo2 >= 70 && spo2 <= 100) {
        gfx->setTextSize(5);
        gfx->setTextColor(COLOR_SPO2);
        char buf[5];
        sprintf(buf, "%d", spo2);
        int16_t x1, y1; uint16_t w, h;
        gfx->getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
        gfx->setCursor((TFT_WIDTH - w) / 2, 155);
        gfx->print(buf);

        gfx->setTextSize(2);
        gfx->setTextColor(COLOR_SPO2);
        gfx->setCursor(TFT_WIDTH - 35, 180);
        gfx->print("%");
    } else {
        gfx->setTextSize(4);
        gfx->setTextColor(COLOR_WARN);
        gfx->setCursor((TFT_WIDTH - 60) / 2, 170);
        gfx->print("--");
    }

    // ───── Signal quality dot (top-right) ─────
    uint16_t dotColor = (validHR && validSpO2) ? COLOR_GOOD : COLOR_WARN;
    gfx->fillCircle(TFT_WIDTH - 20, 20, 10, dotColor);
}