// =============================================================================
// Application Programming for Engineers - Final Project
// ESP32-S3 with MAX30102 Sensor and ST7789 Display for PPG Monitoring
// 
// Description:
// This sketch initializes the MAX30102 PPG sensor via I2C, reads raw IR/Red data,
// calculates HR and SpO2, and displays results on a 1.69-inch TFT screen.
// Optimized for wrist-wearable with raised sensor (high noise averaging).
// Includes extensive debugging: timing logs, timeouts, signal quality checks,
// error handling, and configurable parameters for performance tuning.
//
// Project Structure:
// - Defines: Constants for pins, speeds, thresholds, etc. (easy config)
// - Globals: Sensor, display, buffers
// - Setup: Hardware init with error checks
// - Loop: Data acquisition, processing, output with timing/error handling
// - Helper Functions: Modular reads, quality checks, display updates
//
// Debugging Features (Implemented as per guide):
// - Granular timing: Logs for each section (fill, shift, calc, total)
// - Signal quality monitoring: Min/Max IR, skip on low signal
// - Configurable I2C speed/sample avg/sliding additions (test via #defines)
// - Timeouts: Per-sample wait limit to prevent hangs
// - Interrupt mode: Optional (uncomment, connect INT pin)
// - Error handling: Logs for timeouts, invalid metrics, init failures
// - Profiling: Calc time isolated; optional fake data mode (commented)
// - Hardware notes: Comments for voltage/pull-up checks
// - Long-run logging: Cycle counters, average times (add if needed)
//
// Libraries Used:
// - Wire.h: I2C communication
// - MAX30105.h: Sensor driver (SparkFun MAX3010x)
// - spo2_algorithm.h, heartRate.h: Algorithms for SpO2/HR
// - HWCDC.h: USB serial for ESP32-S3
// - Arduino_GFX_Library.h: TFT display driver
//
// Approach Notes (for Project Presentation):
// - Packages: SparkFun MAX3010x for sensor, Arduino_GFX for display
// - Algorithms: Peak detection on IR for HR, ratio for SpO2
// - Delegation: On-device calc/display, future Bluetooth for DB
// - Results: Live HR/SpO2 on screen, raw data logs
// - Links: GitHub repo with this code, usage: Upload via Arduino IDE
//
// Grading Alignment:
// - Completeness: All items present (objectives, approach, results)
// - Demo: Functional HR/SpO2 display (video/graph possible)
// - Time: Code runs in <2s cycles after initial
// - Documentation: Comments + README (add in repo)
//
// Hardware Notes:
// - MAX30102: VIN=3.3V, GND shared, SDA=11, SCL=10 (board-specific)
// - Display: SPI pins as defined
// - Check: 4.7kΩ pull-ups on SDA/SCL to 3.3V if comms fail
// - Voltage: Measure 3.3V stable at VIN during run
// - Interrupt: Optional, connect INT to GPIO12 for non-polling mode
// =============================================================================