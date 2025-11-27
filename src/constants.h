// constants.h (reduced buffer/size for faster fill, lower thresholds)
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Serial and Delay Config
#define BAUD_RATE 115200
#define SHORT_DELAY 1000

// Logging Config
#define ENABLE_SERIAL_LOGGING true
#define ENABLE_FILE_LOGGING false

// I2C Pins and Speed
#define SDA_PIN 11
#define SCL_PIN 10
#define I2C_SPEED 400000

// Sensor Buffer and Sampling Config
#define MY_BUFFER_SIZE 50            // Reduced for faster initial fill (~1s at 50Hz)
#define SLIDING_ADDITIONS 2          // Reduced for quicker updates
#define SAMPLE_TIMEOUT_MS 1000       // Reduced per-sample wait
#define MIN_IR_THRESHOLD 10000       // Lower for testing
#define MIN_PULSATILE_RANGE 100      // Lower
#define CONSECUTIVE_VALID_REQUIRED 3

// Sensor Hardware Parameters
#define LED_BRIGHTNESS 255           // Max
#define SAMPLE_AVERAGE 32
#define SAMPLE_RATE 50
#define PULSE_WIDTH 411
#define ADC_RANGE 16384
#define LED_MODE 2

// Smoothing Config
#define SMA_WINDOW_SIZE 5
#define HR_HISTORY_SIZE 5
#define MAX_HR_JUMP 20

// Loop and Display Config
#define UPDATE_DELAY_MS 500
#define LCD_DC 4
#define LCD_CS 5
#define LCD_SCK 6
#define LCD_MOSI 7
#define LCD_RST 8
#define LCD_BL 15
#define LCD_WIDTH 240
#define LCD_HEIGHT 280

// Future Expansions
#define MAX_BUFFER_ENTRIES 100

#endif