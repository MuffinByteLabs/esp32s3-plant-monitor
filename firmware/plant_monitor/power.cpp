#include "power.h"

#include <WiFi.h>
#include <esp_sleep.h>
#include <esp_system.h>

#include "adc.h"
#include "config.h"
#include "log.h"
#include "sensors.h"

// Survives deep sleep. I keep last wake's verdict here so the hysteresis has
// something to be hysteretic about — without it a cell sitting exactly on
// 3.50 V would transmit, sag, refuse to transmit, recover, transmit, and so on
// for as long as the reading stayed on the line.
RTC_DATA_ATTR static bool rtcTxAllowed = true;

// A LiPo's voltage-to-charge curve is flat in the middle and steep at both ends,
// so a straight line from 3.0 V to 4.2 V would tell me the cell was half empty
// while it still had two thirds of its energy left. This is the discharge curve
// off the PKCell sheet, sampled at every 50 mV and interpolated between points.
// It is an estimate and I treat it as one: the number I actually make decisions
// on is millivolts.
struct CurvePoint { uint16_t mv; uint8_t pct; };

static const CurvePoint kDischargeCurve[] = {
  {4200, 100}, {4150,  95}, {4100,  90}, {4050,  85}, {4000,  78},
  {3950,  70}, {3900,  62}, {3850,  55}, {3800,  48}, {3750,  42},
  {3700,  35}, {3650,  28}, {3600,  22}, {3550,  17}, {3500,  12},
  {3450,   8}, {3400,   5}, {3300,   2}, {3200,   1}, {3000,   0},
};

static float percentFromMilliVolts(uint32_t mv) {
  const size_t n = sizeof(kDischargeCurve) / sizeof(kDischargeCurve[0]);

  if (mv >= kDischargeCurve[0].mv)     return 100.0f;
  if (mv <= kDischargeCurve[n - 1].mv) return 0.0f;

  for (size_t i = 1; i < n; i++) {
    if (mv >= kDischargeCurve[i].mv) {
      const CurvePoint &hi = kDischargeCurve[i - 1];
      const CurvePoint &lo = kDischargeCurve[i];
      float span = (float)(hi.mv - lo.mv);
      float frac = (float)(mv - lo.mv) / span;
      return lo.pct + frac * (float)(hi.pct - lo.pct);
    }
  }
  return 0.0f;
}

// ---------------------------------------------------------------------------

bool powerUsbHostAttached() {
  return (bool)Serial;
}

BatteryState powerReadBattery() {
  BatteryState b;

  adcConfigurePin(PIN_BAT_SENSE);

  // R14/R15 halve VBAT, so double what the pin sees. BAT_TRIM is the correction
  // I write down at bring-up step C-8 after comparing this against a meter on
  // C16; until I have done that it is 1.0 and this is only as good as the
  // factory ADC calibration and two 1 % resistors, which is roughly a percent.
  uint32_t pinMv = adcAverageMilliVolts(PIN_BAT_SENSE, ADC_SAMPLES);
  b.milliVolts = (uint32_t)((float)pinMv * BAT_DIVIDER_RATIO * BAT_TRIM);
  b.percent    = percentFromMilliVolts(b.milliVolts);

  b.usbHostAttached = powerUsbHostAttached();

  // With USB in and no cell fitted, the charger floats VBAT to about 4.2 V and
  // this reads a beautifully full battery that does not exist. Bring-up section
  // 1 calls this out as expected behaviour of the board, so the firmware has to
  // be the thing that refuses to report it.
  b.trustworthy = !b.usbHostAttached;

  // Hysteresis, applied in one direction only: it takes a clear 3.55 V to earn
  // the radio back, but 3.50 V flat is enough to lose it. I would rather err
  // towards silence.
  uint32_t txThreshold = rtcTxAllowed
                       ? TX_CUTOFF_MV
                       : TX_CUTOFF_MV + CUTOFF_HYSTERESIS_MV;

  b.critical  = (b.milliVolts < SLEEP_CUTOFF_MV) && !b.usbHostAttached;
  b.txAllowed = b.usbHostAttached || (b.milliVolts >= txThreshold);

  rtcTxAllowed = b.txAllowed;
  return b;
}

// ---------------------------------------------------------------------------

const char *powerResetReasonName() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:  return "power-on";
    case ESP_RST_EXT:      return "external";      // SW2
    case ESP_RST_SW:       return "software";
    case ESP_RST_PANIC:    return "panic";
    case ESP_RST_INT_WDT:  return "int-watchdog";
    case ESP_RST_TASK_WDT: return "task-watchdog";
    case ESP_RST_WDT:      return "watchdog";
    case ESP_RST_DEEPSLEEP: return "deep-sleep";   // the normal one
    // The one I am actually watching for. A brown-out here means the USB-unplug
    // hand-over notch is longer than the 50-100 ms R16 was changed to give me,
    // and step 9 of the bring-up guide is where I scope it.
    case ESP_RST_BROWNOUT: return "BROWN-OUT";
    case ESP_RST_SDIO:     return "sdio";
    default:               return "unknown";
  }
}

void powerDeepSleep(uint32_t minutes) {
  // Order matters. The probe goes dark first, because a wake cycle that fails
  // somewhere in the middle still has to not leave it drawing current for the
  // next half hour.
  soilProbePowerOff();
  sensorsSleep();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  LOGF("sleeping for %u min\n", (unsigned)minutes);
#if DEBUG_SERIAL
  Serial.flush();
#endif

  esp_sleep_enable_timer_wakeup((uint64_t)minutes * 60ULL * 1000000ULL);
  esp_deep_sleep_start();
  // Nothing runs after this. The next thing that happens is setup().
}
