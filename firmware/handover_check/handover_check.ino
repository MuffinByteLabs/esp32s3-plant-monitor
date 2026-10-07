// handover_check.ino — Rev A bring-up steps 9 and 10
//
// WHY THIS EXISTS
// bringup_check.ino prints the reset reason once, in setup(). That is fine on
// the bench and useless for step 9, because step 9's event IS pulling USB, and
// this board's serial port is the ESP32-S3's own USB peripheral. Pull USB and
// the COM port vanishes. If the board then browns out and reboots, it prints
// "BROWNOUT" into a port nobody is attached to, and by the time USB is back the
// header has already scrolled past.
//
// So this sketch does two things differently:
//   1. It latches the reset reason at boot and repeats it on EVERY line.
//   2. It prints uptime on every line. Uptime is an independent reset detector
//      that needs no reset-reason register at all: if the number jumped back to
//      about zero while USB was out, the board rebooted. If it kept counting,
//      it rode through. UPTIME IS THE RELIABLE SIGNAL - trust it over the
//      boot counter, which lives in RTC memory and is reloaded from flash by
//      the bootloader on most reset types, so it may read 1 after a reboot
//      instead of incrementing.
//   3. It mirrors everything to UART0 (TP11 = TXD0, TP12 = RXD0, TP8 = GND).
//      Attach a 3.3 V USB-serial adapter there and the log keeps flowing while
//      the board's own USB is unplugged. That is what those pads are for.
//      NEVER connect the adapter's 5 V or 3V3 power pin. TX/RX/GND only.
//
// READING THE RESULT after you plug USB back in: look at the first line.
//   uptime kept climbing, reason unchanged  -> PASS, board rode through
//   uptime restarted near 0, reason BROWNOUT -> FAIL, R16 or gate capacitance
//   uptime restarted near 0, reason POWERON  -> rail collapsed completely
//
// Pins per docs/PinMap_CheatSheet.md. I2C is IO38/IO39 (re-pinned 2026-08-15).

#include <Wire.h>
#include <stdarg.h>
#include "esp_system.h"

#define PIN_SDA           38
#define PIN_SCL           39
#define PIN_ADC_SOIL       1   // ADC1_CH0, direct, no divider
#define PIN_BAT_SENSE      2   // ADC1_CH1, VBAT / 2 via R14/R15
#define PIN_SENS_PWR_EN   21   // Q1 gate. LOW = probe ON, released = off.
#define ADC_SAMPLES       16   // config.h assumes 16. Match it.
#define LINE_INTERVAL_MS 500   // fast enough to catch the moment of the event

// Soil probe. Set to 0 to skip it and get a faster line rate.
// The probe MUST be powered before reading, or R11 pulls ADC_SOIL to ground
// and you read ~0 regardless of what is in the pot. LOW on Q1 = probe ON.
#define SOIL_ENABLED       1
#define SOIL_SETTLE_MS   250   // matches sensor_check.ino

// With "USB CDC On Boot" enabled, Serial is the USB port and Serial0 is UART0.
// With it disabled, Serial already IS UART0 and mirroring would double-print.
#if ARDUINO_USB_CDC_ON_BOOT
  #define MIRROR_TO_UART0 1
#else
  #define MIRROR_TO_UART0 0
#endif

RTC_DATA_ATTR static uint32_t bootCount = 0;   // survives most resets

static esp_reset_reason_t g_resetReason;
static const char *g_resetName;

static const char *resetReasonName(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "POWERON";
    case ESP_RST_EXT:       return "EXT";
    case ESP_RST_SW:        return "SW";
    case ESP_RST_PANIC:     return "PANIC";
    case ESP_RST_INT_WDT:   return "INT_WDT";
    case ESP_RST_TASK_WDT:  return "TASK_WDT";
    case ESP_RST_WDT:       return "WDT";
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
    case ESP_RST_BROWNOUT:  return "BROWNOUT";
    case ESP_RST_SDIO:      return "SDIO";
    default:                return "UNKNOWN";
  }
}

// Print to the USB port and to UART0 at the same time.
static void out(const char *fmt, ...) {
  char buf[220];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  Serial.print(buf);
#if MIRROR_TO_UART0
  Serial0.print(buf);
#endif
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

void setup() {
  // Park the probe off at boot: released, so R10's pull-up holds Q1's gate
  // high. loop() powers it on demand when SOIL_ENABLED.
  pinMode(PIN_SENS_PWR_EN, INPUT);

  // Latch the reset reason FIRST, before anything can disturb it.
  g_resetReason = esp_reset_reason();
  g_resetName   = resetReasonName(g_resetReason);
  bootCount++;

  Serial.begin(115200);
#if MIRROR_TO_UART0
  Serial0.begin(115200);   // TP11 = TXD0 (GPIO43), TP12 = RXD0 (GPIO44)
#endif

  // Do NOT block forever waiting for a host: on battery there is no host.
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) { delay(10); }
  delay(100);

  out("\n=================================================\n");
  out(" Rev A hand-over check - bring-up steps 9 and 10\n");
  out("=================================================\n");
  out("  Reset reason : %s\n", g_resetName);
  out("  Boot count   : %u (best effort - uptime is the reliable signal)\n",
      (unsigned)bootCount);
  out("  Chip         : %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  if (g_resetReason == ESP_RST_BROWNOUT) {
    out("  *** BROWNOUT - this is the step 9 FAILURE signature ***\n");
  }
  out("\n");

  Wire.begin(PIN_SDA, PIN_SCL, 100000UL);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_ADC_SOIL,  ADC_11db);
  analogSetPinAttenuation(PIN_BAT_SENSE, ADC_11db);

  out("  up(s) | reset    | boot | VBAT     | BAT raw | SOIL raw\n");
  out("  ------+----------+------+----------+---------+---------\n");
}

void loop() {
  // Battery first, with the probe still off, so the probe's current draw
  // cannot disturb the reading.
  uint32_t batRaw = readAvgRaw(PIN_BAT_SENSE);
  uint32_t batMv  = readAvgMv(PIN_BAT_SENSE);

  // Soil: power the probe, let it settle, average, then release the gate.
  // 16 samples here, unlike sensor_check.ino's single sample - this is the
  // averaging config.h assumes, so these numbers are calibration-grade.
  uint32_t soRaw = 0;
#if SOIL_ENABLED
  pinMode(PIN_SENS_PWR_EN, OUTPUT);
  digitalWrite(PIN_SENS_PWR_EN, LOW);      // LOW = probe ON
  delay(SOIL_SETTLE_MS);
  soRaw = readAvgRaw(PIN_ADC_SOIL);
  pinMode(PIN_SENS_PWR_EN, INPUT);         // release; R10 holds Q1 off
#endif

  // Uptime is the reset detector that needs no register: watch this number.
  // Formatted with integer math on purpose - no %f, so it cannot be defeated
  // by a printf build without float support.
  uint32_t ms = millis();
  out("  %4u.%01u | %-8s | %4u | %4u mV  | %7u | %7u\n",
      (unsigned)(ms / 1000), (unsigned)((ms % 1000) / 100),
      g_resetName, (unsigned)bootCount,
      (unsigned)(batMv * 2), (unsigned)batRaw, (unsigned)soRaw);

  delay(LINE_INTERVAL_MS);
}
