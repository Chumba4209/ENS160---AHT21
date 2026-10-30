#include "select.h"
#if ACTIVE_TEST == 1

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 8
#define SCL_PIN 10
#define OLED_ADDR 0x3C
#define AHT_ADDR 0x38

Adafruit_SSD1306 display(128, 64, &Wire, -1);

float temp = NAN, hum = NAN;
bool lastReadOk = false;

void scan() {
  Serial.print("I2C scan: ");
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) Serial.printf("0x%02X ", a);
  }
  Serial.println();
}

// Direct AHT2x read, no library. Returns true on success.
bool readAHT() {
  // Trigger measurement
  Wire.beginTransmission(AHT_ADDR);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission() != 0) return false;

  delay(100);  // measurement takes ~80 ms

  // Read 7 bytes: status, 5 data bytes, CRC
  if (Wire.requestFrom((uint8_t)AHT_ADDR, (uint8_t)7) != 7) return false;
  uint8_t d[7];
  for (int i = 0; i < 7; i++) d[i] = Wire.read();

  if (d[0] & 0x80) return false;  // still busy

  uint32_t rawH = ((uint32_t)d[1] << 12) | ((uint32_t)d[2] << 4) | (d[3] >> 4);
  uint32_t rawT = ((uint32_t)(d[3] & 0x0F) << 16) | ((uint32_t)d[4] << 8) | d[5];

  hum  = rawH * 100.0f / 1048576.0f;
  temp = rawT * 200.0f / 1048576.0f - 50.0f;
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 not found!");
    while (true) delay(1000);
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  scan();

  // Read the AHT status byte for diagnostics (expect bit 3 = calibrated)
  Wire.requestFrom((uint8_t)AHT_ADDR, (uint8_t)1);
  if (Wire.available()) {
    uint8_t s = Wire.read();
    Serial.printf("AHT status byte: 0x%02X (calibrated bit %s)\n",
                  s, (s & 0x08) ? "set" : "NOT set");
  } else {
    Serial.println("AHT status byte: no response");
  }
}

void loop() {
  lastReadOk = readAHT();

  if (lastReadOk) {
    Serial.printf("T: %.1f C | H: %.1f %%\n", temp, hum);
  } else {
    Serial.println("AHT read failed");
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AHT21 raw test");
  if (lastReadOk) {
    display.setCursor(0, 20);  display.printf("Temp : %.1f C", temp);
    display.setCursor(0, 35);  display.printf("Humid: %.1f %%", hum);
  } else {
    display.setCursor(0, 20);  display.println("Read failed");
  }
  display.display();

  delay(2000);
}

#endif