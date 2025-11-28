// constants.h
// ESP32-S3 Wearable Biometric Watch – Reliable Production Settings
// Updated: November 28, 2025 by Jason Sharer + community best practices
#ifndef CONSTANTS_H
#define CONSTANTS_H

// ====================== Serial & Timing ======================
#define BAUD_RATE               115200
#define SHORT_DELAY             1000      // Initial stabilization delay
#define UPDATE_DELAY_MS         200       // Main loop delay → ~5 FPS update (feels smooth, saves power)

// ====================== Logging Control ======================
#define ENABLE_SERIAL_LOGGING   true      // Keep true during dev, false in final product
#define ENABLE_FILE_LOGGING     false     // Set true only for long-term data collection (flash killer otherwise!)

// ====================== I2C Pins & Speed ======================
#define SENSOR_SDA_PIN          11
#define SENSOR_SCL_PIN          10
#define SENSOR_I2C_SPEED        400000    // 400 kHz – MAX30102 officially supports it

// ====================== MAX30102 Sensor Config ======================
// These are the "gold standard" settings used by Maxim, SparkFun, and real products
#define MY_BUFFER_SIZE          50       // 100 samples = 1 second at 100 Hz → perfect for algorithm
#define SLIDING_ADDITIONS       12        // Update with 25 new samples → new reading every ~250 ms

#define SAMPLE_RATE             100       // 100 Hz – best balance of accuracy and power
#define SAMPLE_AVERAGE          4         // 4-sample average inside sensor
#define LED_MODE                2         // 2 LEDs (Red + IR)
#define PULSE_WIDTH             215       // 411 µs → 18-bit resolution
#define ADC_RANGE               4096      // 11.0 pA per LSB – standard for wearable

#define LED_BRIGHTNESS_DEFAULT  60        // Start at ~6 mA – safe, good signal, low heat
#define LED_BRIGHTNESS_MAX      255       // 50 mA max (0xFF)

// ====================== Signal Quality Thresholds ======================
// These are REAL values from finger-on-sensor testing (not bench/no-finger)
#define MIN_IR_THRESHOLD        50000     // Below this = no finger or very poor contact
#define MIN_PULSATILE_RANGE     3000      // AC component must be at least this big
#define CONSECUTIVE_VALID_REQUIRED 1      // Need 3 good cycles in a row before trusting

// Timeout for reading one sample (prevents hard lockups)
#define SAMPLE_TIMEOUT_MS       100

// ====================== Smoothing & Stability ======================
#define SMA_WINDOW_SIZE         5         // Moving average on raw samples
#define HR_HISTORY_SIZE         8         // Average last 8 valid HR readings
#define MAX_HR_JUMP             20        // Reject jumps >18 bpm (physiological limit ~15–20 bpm per beat)

// ====================== Display (ST7789 240x280) ======================
#define LCD_DC                  4
#define LCD_CS                  5
#define LCD_SCK                 6
#define LCD_MOSI                7
#define LCD_RST                 8
#define LCD_BL                  15
#define LCD_WIDTH               240
#define LCD_HEIGHT              280

// Optional rotation offset if your display is upside-down
// #define ROTATION              1   // Uncomment and change if needed

// ====================== Future Use ======================
#define MAX_BUFFER_ENTRIES      500       // For long-term logging if ever needed

#endif