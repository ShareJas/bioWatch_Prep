// display.cpp — FINAL BEAUTIFUL VERSION FOR 240×280
#include "display.h"

#define COLOR_BG      0x0000
#define COLOR_TEXT    0xFFFF
#define COLOR_HR      0xF800   // Red
#define COLOR_SPO2    0x001F   // Blue
#define COLOR_WARN    0xFD20   // Orange
#define COLOR_DIM     0x7BEF   // Light gray

void DisplayManager::init() {
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI, -1);
    gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, 240, 280);  // 240×280

    gfx->begin();
    gfx->fillScreen(COLOR_BG);
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    // Beautiful startup screen
    gfx->setTextColor(0x07E0); // Green
    gfx->setTextSize(3);
    gfx->setCursor(45, 80);
    gfx->print("BIOWATCH");

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_DIM);
    gfx->setCursor(20, 140);
    gfx->print("Place finger on sensor");

    delay(2000);
    gfx->fillScreen(COLOR_BG);
}

void DisplayManager::updateMetrics(int32_t hr, int32_t spo2, bool validHR, bool validSpO2) {
    gfx->fillScreen(COLOR_BG);  // Full clear for clean look

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(20, 20);
    gfx->print("Heart Rate");

    if (validHR && hr >= 40 && hr <= 200) {
        gfx->setTextSize(6);
        gfx->setTextColor(COLOR_HR);
        gfx->setCursor(30, 70);
        gfx->printf("%d", hr);
        gfx->setTextSize(3);
        gfx->setCursor(170, 100);
        gfx->print("BPM");
    } else {
        gfx->setTextSize(5);
        gfx->setTextColor(COLOR_WARN);
        gfx->setCursor(50, 80);
        gfx->print("--");
    }

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(20, 160);
    gfx->print("SpO2");

    if (validSpO2 && spo2 >= 70 && spo2 <= 100) {
        gfx->setTextSize(6);
        gfx->setTextColor(COLOR_SPO2);
        gfx->setCursor(50, 200);
        gfx->printf("%d", spo2);
        gfx->setTextSize(3);
        gfx->setCursor(170, 225);
        gfx->print("%");
    } else {
        gfx->setTextSize(5);
        gfx->setTextColor(COLOR_WARN);
        gfx->setCursor(50, 200);
        gfx->print("--");
    }

    // Signal quality indicator
    uint16_t dotColor = (validHR && validSpO2) ? 0x07E0 : 0xF800;
    gfx->fillCircle(220, 20, 12, dotColor);
}