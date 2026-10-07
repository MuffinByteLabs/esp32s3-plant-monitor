// sensor_check.ino — basic Rev A sensor read (temperature, humidity, pressure, light)
//
// Bring-up helper. Stays awake so the native USB CDC port stays alive.

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML7700.h>

// Pins from docs/PinMap_CheatSheet.md.
// I2C was re-pinned IO4/IO5 -> IO38/IO39 on 2026-08-15. Wire.begin() takes SDA first.
#define SDA_PIN   38
#define SCL_PIN   39
#define SOIL_PIN   1   // GPIO1, ADC1_CH0, direct from probe (no divider)
#define BAT_PIN    2   // GPIO2, ADC1_CH1, VBAT/2 via R14/R15
#define SOIL_PWR  21   // Q1 gate. LOW = probe ON. R10 holds it off otherwise.

#define SOIL_SETTLE_MS 250

Adafruit_BME280 bme;
Adafruit_VEML7700 veml;

bool bmeFound  = false;
bool vemlFound = false;
uint8_t bmeAddress = 0;

void i2cScan() {
  Serial.println("I2C scan (SDA=38, SCL=39):");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      found++;
      Serial.print("  0x");
      Serial.print(addr, HEX);
      if      (addr == 0x76) Serial.println("  BME280");
      else if (addr == 0x10) Serial.println("  VEML7700");
      else                   Serial.println("  unexpected");
    }
  }
  if (found == 0) {
    Serial.println("  nothing on the bus - check R8/R9 pull-ups and 3V3 at the sensors");
  }
}

void setup() {
  // Probe off: released, so R10's pull-up holds Q1's gate high.
  pinMode(SOIL_PWR, INPUT);

  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 3000) { delay(10); }
  delay(200);

  Serial.println();
  Serial.println("PROGRAM STARTED");

  Wire.begin(SDA_PIN, SCL_PIN, 100000UL);

  // BME280 is strapped to 0x76 on this board (SDO -> GND, CSB -> 3V3).
  // 0x77 is tried only as a diagnostic - finding it there means SDO is floating.
  if (bme.begin(0x76, &Wire)) {
    bmeFound = true;
    bmeAddress = 0x76;
  } else if (bme.begin(0x77, &Wire)) {
    bmeFound = true;
    bmeAddress = 0x77;
    Serial.println("WARNING: BME280 answered at 0x77, expected 0x76. Check SDO.");
  }

  vemlFound = veml.begin(&Wire);

  if (bmeFound) {
    Serial.print("BME280 found at 0x");
    Serial.println(bmeAddress, HEX);
  } else {
    Serial.println("BME280 NOT FOUND");
  }

  if (vemlFound) Serial.println("VEML7700 found");
  else           Serial.println("VEML7700 NOT FOUND");

  if (!bmeFound || !vemlFound) {
    Serial.println();
    i2cScan();
  }

  analogReadResolution(12);
  analogSetPinAttenuation(SOIL_PIN, ADC_11db);
  analogSetPinAttenuation(BAT_PIN,  ADC_11db);

  Serial.println();
}

void loop() {
  if (bmeFound) {
    Serial.print("Temperature: ");
    Serial.print(bme.readTemperature());
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.print("Pressure: ");
    Serial.print(bme.readPressure() / 100.0);
    Serial.println(" hPa");
  }

  if (vemlFound) {
    Serial.print("Light: ");
    Serial.print(veml.readLux());
    Serial.println(" lux");
  }

  // Soil: power the probe, let it settle, read, then release the gate.
  pinMode(SOIL_PWR, OUTPUT);
  digitalWrite(SOIL_PWR, LOW);          // LOW = probe ON
  delay(SOIL_SETTLE_MS);
  int soilRaw = analogRead(SOIL_PIN);
  pinMode(SOIL_PWR, INPUT);             // release; R10 holds Q1 off

  Serial.print("Soil raw: ");
  Serial.print(soilRaw);
  Serial.println("   (near 0 is correct with no probe in J2)");

  // Free, and it pre-loads bring-up step C-8. Ignore until a battery is fitted:
  // with USB in and no cell, the charger floats VBAT and this reads "full".
  int batRaw = analogRead(BAT_PIN);
  Serial.print("BAT_SENSE raw: ");
  Serial.print(batRaw);
  Serial.print("  -> VBAT ~");
  Serial.print(analogReadMilliVolts(BAT_PIN) * 2);
  Serial.println(" mV");

  Serial.println("----------------");
  delay(1000);
}
