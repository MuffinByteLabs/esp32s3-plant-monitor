// bringup_check.ino — Rev A hardware bring-up helper
//
// Covers BringUp_Guide.md steps 4, 5 and 6 in one flash, and pre-loads the
// numbers step C-8 needs. Deliberately DOES NOT sleep: the USB port on this
// board is the ESP32-S3's own USB Serial/JTAG peripheral, not a bridge chip,
// so a board in deep sleep has no COM port and every upload after that needs
// the hold-BOOT / tap-RESET dance. This sketch stays awake and keeps the port.
//
// There is no GPIO-controlled LED on Rev A — D2 is hardwired to +3V3 through
// R5 and D3 belongs to the charger — so "is it alive?" is answered over serial.
//
// Pins from docs/PinMap_CheatSheet.md (I2C re-pinned to IO38/IO39 2026-08-15).

#include <Wire.h>
#include "esp_system.h"

#define PIN_SDA          38
#define PIN_SCL          39
#define PIN_ADC_SOIL      1   // ADC1_CH0, direct, no divider
#define PIN_BAT_SENSE     2   // ADC1_CH1, VBAT / 2 via R14/R15
#define PIN_SENS_PWR_EN  21   // LOW = probe on. Left released here.
#define ADC_SAMPLES      16

static const char *resetReasonName(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:  return "POWERON  (cold start / battery connect)";
    case ESP_RST_EXT:      return "EXT      (RESET button, SW2)";
    case ESP_RST_SW:       return "SW       (software restart)";
    case ESP_RST_PANIC:    return "PANIC    (exception / crash)";
    case ESP_RST_INT_WDT:  return "INT_WDT  (interrupt watchdog)";
    case ESP_RST_TASK_WDT: return "TASK_WDT (task watchdog)";
    case ESP_RST_WDT:      return "WDT      (other watchdog)";
    case ESP_RST_DEEPSLEEP:return "DEEPSLEEP(woke from deep sleep)";
    case ESP_RST_BROWNOUT: return "BROWNOUT <-- THIS IS THE STEP 9 FAILURE";
    case ESP_RST_SDIO:     return "SDIO";
    default:               return "UNKNOWN";
  }
}

static uint32_t readAvgRaw(int pin) {
  uint32_t acc = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) { acc += analogRead(pin); delay(2); }
  return acc / ADC_SAMPLES;
}

static uint32_t readAvgMv(int pin) {
  uint32_t acc = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) { acc += analogReadMilliVolts(pin); delay(2); }
  return acc / ADC_SAMPLES;
}

void i2cScan() {
  Serial.println(F("  I2C scan on SDA=38 SCL=39:"));
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      found++;
      Serial.printf("    0x%02X  <-- %s\n", addr,
        addr == 0x76 ? "BME280   (expected)" :
        addr == 0x10 ? "VEML7700 (expected)" : "UNEXPECTED DEVICE");
    }
  }
  if (!found) Serial.println(F("    NOTHING FOUND. Check R8/R9 pull-ups, U4 SDO->GND, U5 solder."));
  else {
    Serial.printf("    %d device(s). Step 5 passes only with BOTH 0x76 and 0x10.\n", found);
  }
}

void setup() {
  // Probe stays off: released, so R10's pull-up holds Q1's gate high.
  pinMode(PIN_SENS_PWR_EN, INPUT);

  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 3000) { delay(10); }
  delay(200);

  Serial.println();
  Serial.println(F("================================================"));
  Serial.println(F(" ESP32-S3 Plant Monitor Rev A - bring-up check"));
  Serial.println(F("================================================"));

  esp_reset_reason_t r = esp_reset_reason();
  Serial.printf("  Reset reason : %s\n", resetReasonName(r));
  Serial.printf("  Chip         : %s rev %d, %d core(s)\n",
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("  Flash        : %u MB  (expect 8 for WROOM-1-N8)\n",
                (unsigned)(ESP.getFlashChipSize() / (1024 * 1024)));
  Serial.printf("  PSRAM        : %u bytes (expect 0 on N8)\n", (unsigned)ESP.getPsramSize());
  Serial.println();

  Wire.begin(PIN_SDA, PIN_SCL, 100000UL);
  i2cScan();
  Serial.println();

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_ADC_SOIL,  ADC_11db);
  analogSetPinAttenuation(PIN_BAT_SENSE, ADC_11db);

  Serial.println(F("  Streaming ADC every 2 s. Ctrl-click reset (SW2) to re-run header."));
  Serial.println();
}

void loop() {
  uint32_t batRaw = readAvgRaw(PIN_BAT_SENSE);
  uint32_t batMv  = readAvgMv(PIN_BAT_SENSE);
  uint32_t soRaw  = readAvgRaw(PIN_ADC_SOIL);
  uint32_t soMv   = readAvgMv(PIN_ADC_SOIL);

  Serial.printf("BAT_SENSE %4u raw  %4u mV  -> VBAT ~%u mV   |   ADC_SOIL %4u raw  %4u mV\n",
                (unsigned)batRaw, (unsigned)batMv, (unsigned)(batMv * 2),
                (unsigned)soRaw, (unsigned)soMv);
  delay(2000);
}
