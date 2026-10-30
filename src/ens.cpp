#include "select.h"
#if ACTIVE_TEST == 2

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_ENS160.h>

#define SDA_PIN 8
#define SCL_PIN 10
#define ENS_ADDR 0x52

SparkFun_ENS160 ens160;
bool ensOk = false;

void scan(const char *label) {
  Serial.printf("[%s] scan: ", label);
  int n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { Serial.printf("0x%02X ", a); n++; }
  }
  if (!n) Serial.print("nothing");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(200);

  scan("start");

  ensOk = ens160.begin(ENS_ADDR);
  Serial.println(ensOk ? "ENS160 begin OK" : "ENS160 begin FAILED");

  if (ensOk) {
    ens160.setOperatingMode(SFE_ENS160_RESET);
    delay(100);
    ens160.setOperatingMode(SFE_ENS160_STANDARD);
    delay(500);
  }
  scan("after ENS160 init");
}

void loop() {
  delay(2000);

  if (!ensOk) {
    Serial.println("ENS160 not initialised");
    return;
  }

  int flags = ens160.getFlags();          // restored: refreshes the status
  bool fresh = ens160.checkDataStatus();

  Serial.printf("flag=%d new=%d | AQI: %d | TVOC: %d ppb | eCO2: %d ppm\n",
                flags, fresh,
                ens160.getAQI(), ens160.getTVOC(), ens160.getECO2());
}

#endif