#include <Wire.h>
#include "MAX30105.h"  // SparkFun library for MAX30102
#include "spo2_algorithm.h"
#include "heartRate.h"  // For calculations
#include "HWCDC.h"     // For USB serial on ESP32-S3
#include <Arduino_GFX_Library.h>  // For display

// Display pins from your old code
#define LCD_DC 4
#define LCD_CS 5
#define LCD_SCK 6
#define LCD_MOSI 7
#define LCD_RST 8
#define LCD_BL 15
#define LCD_WIDTH 240
#define LCD_HEIGHT 280

// I2C setup
#define SDA 11
#define SCL 10
#define I2C_SPEED 50000

// Serial and delays
#define BAUD_RATE 115200
#define SHORT_DELAY 1000

// Buffer Size
#define BUFFER_SIZE  100

HWCDC USBSerial;          // USB serial
MAX30105 particleSensor;  // MAX30102

const int bufferSize = 100;  // ~1 sec at 100 Hz
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int32_t heartRate;
int8_t validSpo2;
int8_t validHeartRate;
unsigned long startTime;

// Display setup from old code
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

void setup() {
  USBSerial.begin(BAUD_RATE);
  while (!USBSerial);
  delay(SHORT_DELAY);

  USBSerial.println("Debug: Before Wire.begin()");
  Wire.begin(SDA, SCL);
  Wire.setClock(I2C_SPEED);
  USBSerial.println("Debug: After Wire.begin()");

  USBSerial.println("Debug: Attempting sensor init");
  if (!particleSensor.begin(Wire, I2C_SPEED)) {
    USBSerial.println("Error: MAX30102 init failed. Check wiring/power/address (0x57).");
    while (1);
  }
  USBSerial.println("Sensor initialized!");

  // Sensor config (wrist-optimized)
  byte ledBrightness = 80;
  byte sampleAverage = 16;  // High for raised noise
  byte ledMode = 2;         // Red + IR
  int sampleRate = 100;     // Hz
  int pulseWidth = 411;     // Max SNR
  int adcRange = 8192;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  USBSerial.println("Sensor configured. Place on skin for PPG data.");

  // Init display
  if (!gfx->begin()) {
    USBSerial.println("Display init failed!");
    while (1);
  }
  gfx->fillScreen(BLACK);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  USBSerial.println("Display ready.");
}

void loop() {
  startTime = millis();  // Start timing

  // Initial full buffer fill (only once)
  static bool firstRun = true;
  if (firstRun) {
    for (int i = 0; i < bufferSize; i++) {
      while (!particleSensor.available()) particleSensor.check();
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
    }
    firstRun = false;
    USBSerial.println("Initial buffer filled.");
  } else {
    // Sliding window: Shift and add 25 new samples (~0.25 sec update)
    for (int i = 0; i < 25; i++) {
      // Shift buffers left
      memmove(redBuffer, redBuffer + 1, (bufferSize - 1) * sizeof(uint32_t));
      memmove(irBuffer, irBuffer + 1, (bufferSize - 1) * sizeof(uint32_t));

      // Add new sample
      while (!particleSensor.available()) particleSensor.check();
      redBuffer[bufferSize - 1] = particleSensor.getRed();
      irBuffer[bufferSize - 1] = particleSensor.getIR();
      particleSensor.nextSample();
    }
  }

  // Calc HR/SpO2
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferSize, redBuffer, &spo2, &validSpo2, &heartRate, &validHeartRate);

  // Timing log
  unsigned long calcTime = millis() - startTime;
  USBSerial.print("Cycle time: ");
  USBSerial.print(calcTime);
  USBSerial.println(" ms");

  // Stream raw sample
  USBSerial.print("Raw PPG - IR: ");
  USBSerial.print(irBuffer[bufferSize - 1]);
  USBSerial.print(", Red: ");
  USBSerial.println(redBuffer[bufferSize - 1]);

  // Output metrics to serial
  USBSerial.print(validHeartRate ? "HR: " + String(heartRate) + " bpm" : "Invalid HR");
  USBSerial.print(", ");
  USBSerial.println(validSpo2 ? "SpO2: " + String(spo2) + "%" : "Invalid SpO2");

  // Display metrics (update text without full clear for speed)
  gfx->fillRect(10, 10, 200, 60, BLACK);  // Clear small area
  gfx->setCursor(10, 10);
  gfx->setTextColor(RED);
  gfx->setTextSize(2);
  gfx->println(validHeartRate ? "HR: " + String(heartRate) : "No HR");
  gfx->setCursor(10, 40);
  gfx->println(validSpo2 ? "SpO2: " + String(spo2) : "No SpO2");

  if (irBuffer[bufferSize - 1] < 50000) {
    USBSerial.println("Low signal - Check contact");
  }

  delay(250);  // Shorter delay for faster cycles
}