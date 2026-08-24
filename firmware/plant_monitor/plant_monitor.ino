// ESP32-S3 Plant Monitor - Rev A firmware
// Ray Malik - muffinbytelabs.com
//
// One wake cycle, start to finish. There is no loop in the usual sense: the
// board wakes from deep sleep, runs setup() once, and goes back to sleep at the
// end of it. Everything after esp_deep_sleep_start() is unreachable by design,
// so the whole program is the order of operations below and nothing else.
//
//   1. Read the battery, before anything expensive happens.
//   2. If the cell is under 3.0 V, do nothing else at all and sleep long.
//   3. Switch the soil probe on and start its 250 ms settle clock.
//   4. Read the two I2C sensors while that clock runs - it is time I have to
//      spend anyway, so I would rather spend it working.
//   5. Read the probe, switch it off.
//   6. If the cell has the margin for it, bring the radio up and publish.
//   7. Sleep.
//
// The order is deliberate. The battery gate comes first because the decision it
// makes is whether the rest of the cycle is allowed to happen; the probe comes
// on before the I2C reads because its settle time is the longest thing here and
// it can overlap with something useful; the radio comes last because it is the
// only part that draws current in the hundreds of milliamps and I want the cell
// to have had no other load on it when I measured it.
//
// The board's side of this contract - what the hardware requires and where each
// number comes from - is in firmware/README.md. The numbers themselves are in
// config.h.

#include <Arduino.h>

#include "adc.h"
#include "config.h"
#include "log.h"
#include "net.h"
#include "power.h"
#include "sensors.h"

// The RTC domain keeps its contents through deep sleep, which is the only
// memory that survives a wake here. Everything else starts from zero every
// half hour.
RTC_DATA_ATTR static uint32_t bootCount = 0;

static uint32_t wakeStartMs = 0;

// A wake cycle that overruns is a wake cycle burning 80 mA on a cell I am
// trying to make last a hundred days. Each stage below checks this before
// starting, and any stage that finds the deadline gone gives up and sleeps
// rather than beginning something new. Individual network operations have their
// own shorter timeouts inside net.cpp; this is the backstop over all of them.
static bool deadlineReached() {
  return (millis() - wakeStartMs) > AWAKE_DEADLINE_MS;
}

void setup() {
  wakeStartMs = millis();
  bootCount++;

  logBegin();

  LOGLN();
  LOGF("=== %s  fw %s  boot %u  reset: %s ===\n",
       NODE_MODEL, FIRMWARE_VERSION, (unsigned)bootCount, powerResetReasonName());

  analogReadResolution(12);
  adcConfigurePin(PIN_ADC_SOIL);
  adcConfigurePin(PIN_BAT_SENSE);

  // --- 1. Battery -------------------------------------------------------

  BatteryState battery = powerReadBattery();

  if (battery.trustworthy) {
    LOGF("battery: %u mV (%.0f%%)%s\n", (unsigned)battery.milliVolts, battery.percent,
         battery.txAllowed ? "" : "  - below the TX cutoff, radio stays off");
  } else {
    LOGF("battery: %u mV, not reported - a USB host is attached and the charger "
         "is holding VBAT up\n", (unsigned)battery.milliVolts);
  }

  // --- 2. The 3.0 V floor ----------------------------------------------

  if (battery.critical) {
    // PKCell's discharge floor. Below this I take no readings, transmit
    // nothing, and go straight back to sleep on the long interval. The pack's
    // own protection module will not disconnect until 2.5 V and by then the
    // cell is damaged, so this line is the only thing standing between a flat
    // battery and a ruined one.
    //
    // I still wake, though - just rarely. A board that stopped waking entirely
    // could never notice it had been put on the charger.
    LOGF("battery: %u mV is below the %d mV floor - sleeping %d min\n",
         (unsigned)battery.milliVolts, SLEEP_CUTOFF_MV, WAKE_INTERVAL_CRITICAL_MIN);
    powerDeepSleep(WAKE_INTERVAL_CRITICAL_MIN);
  }

  // --- 3, 4, 5. Sensors -------------------------------------------------

  Readings readings;

  soilProbePowerOn();

  if (sensorsBegin()) {
    sensorsReadClimateAndLight(readings);
  } else {
    LOGLN("i2c: nothing answered on the bus - check R8/R9 and the 3V3 rail");
  }

  sensorsReadSoil(readings);
  soilProbePowerOff();

  if (readings.soilValid) {
    LOGF("soil:    %d raw  (%.1f%%)\n", readings.soilRaw, readings.soilPct);
  }
  if (readings.climateValid) {
    LOGF("climate: %.2f C  %.1f %%RH  %.1f hPa\n",
         readings.temperatureC, readings.humidityPct, readings.pressureHpa);
  }
  if (readings.lightValid) {
    LOGF("light:   %.1f lx\n", readings.lux);
  }

  // --- 6. Transmit ------------------------------------------------------

  if (!battery.txAllowed) {
    // Not a battery rule but a regulator one: the AP2112K needs about 200 mV of
    // headroom to hold 3.3 V through a 355 mA transmit peak, and below roughly
    // 3.5 V it does not have it. Reading sensors and sleeping are still fine
    // down here - it is only the radio that is banned - so the readings above
    // still happened, they just have nowhere to go this cycle.
    LOGLN("tx: skipped, cell is under the regulator's headroom");

  } else if (deadlineReached()) {
    LOGLN("tx: skipped, this wake has already run long");

  } else if (netConnectWifi() && !deadlineReached()) {
    if (netConnectBroker(bootCount)) {
      netPublishState(readings, battery, bootCount);
    }
    netShutdown();

  } else {
    netShutdown();
  }

  // --- 7. Sleep ---------------------------------------------------------

  LOGF("awake for %u ms\n", (unsigned)(millis() - wakeStartMs));
  powerDeepSleep(WAKE_INTERVAL_MIN);
}

void loop() {
  // Unreachable. powerDeepSleep() does not return, and setup() always ends in
  // it. If execution ever does arrive here, something has gone wrong that I do
  // not understand - so rather than spin at full power, say so and sleep.
  LOGLN("loop() reached, which should not happen - sleeping anyway");
  powerDeepSleep(WAKE_INTERVAL_MIN);
}
