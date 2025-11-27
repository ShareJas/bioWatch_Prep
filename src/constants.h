//-----------------------------------
// ESP-32 Wearable Biometric Watch
// constants.h
// Written 11.27.25 
// Modified by Jason Sharer
//-----------------------------------

// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Serial and Delay Config
#define BAUD_RATE 115200             // Serial baud rate for debugging (unused after switching to BLE; standard value for fast comms without errors)
#define SHORT_DELAY 1000             // ms delay after init for hardware stabilization (reduce to 500 if boot feels slow)

// I2C Pins and Speed (Board-Specific for Waveshare ESP32-S3 Touch LCD 1.69)
#define SDA_PIN 11                   // I2C data pin; shared with touch/RTC/IMU on this board
#define SCL_PIN 10                   // I2C clock pin; shared as above
#define I2C_SPEED 400000             // I2C clock speed in Hz (400kHz fast; drop to 100000 if comms flaky due to long wires/noise)

// Sensor Buffer and Sampling Config
#define MY_BUFFER_SIZE 100           // Number of samples in rolling buffer (~1s at 100Hz effective rate; smaller=50 for faster loops, larger=200 for smoother HR/SpO2)
#define SLIDING_ADDITIONS 10         // New samples added per loop cycle (balances update frequency vs. CPU load; 10=~0.1s updates)
#define SAMPLE_TIMEOUT_MS 5000       // Max ms wait for a single sensor sample (prevents infinite loops on hardware failure)
#define MIN_IR_THRESHOLD 50000       // Minimum IR value for valid signal (below this = poor skin contact or noise; tune via testing, e.g., 30000 for darker skin or low light)

// Sensor Hardware Parameters (Optimized for MAX30102 on Wrist)
#define LED_BRIGHTNESS 100           // LED intensity (0-255; higher=better signal but more battery drain; consider dynamic adjustment later)
#define SAMPLE_AVERAGE 16            // Number of internal averages per sample (1-32; higher=smoother but slower readout)
#define SAMPLE_RATE 400              // Sampling frequency in Hz (50-3200; 400 is good balance; lower=100 for battery saving)
#define PULSE_WIDTH 411              // LED pulse width in us (69-411; max=411 for best SNR in noisy environments)
#define ADC_RANGE 8192               // ADC resolution (2048-16384; higher=more precision but slight perf hit)
#define LED_MODE 2

// Loop and Display Config
#define UPDATE_DELAY_MS 250          // ms delay between loop cycles (shorter=100 for more real-time feel, but higher CPU/battery use)
#define LCD_DC 4                     // Display data/command pin (SPI control)
#define LCD_CS 5                     // Display chip select pin
#define LCD_SCK 6                    // Display clock pin
#define LCD_MOSI 7                   // Display data out pin
#define LCD_RST 8                    // Display reset pin
#define LCD_BL 15                    // Display backlight pin
#define LCD_WIDTH 240                // Display resolution width in pixels
#define LCD_HEIGHT 280               // Display resolution height in pixels

// Future Expansions (Added for Later Steps)
#define MAX_BUFFER_ENTRIES 100       // Size of timestamped data log buffer (for RTC integration)

#endif