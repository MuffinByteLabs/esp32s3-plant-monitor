#include "sensors.h"

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML7700.h>

#include "adc.h"
#include "config.h"
#include "log.h"

static Adafruit_BME280   bme;
static Adafruit_VEML7700 veml;

static bool     bmePresent  = false;
static bool     vemlPresent = false;
static bool     busStarted  = false;
static uint32_t probeOnAtMs = 0;

// ---------------------------------------------------------------------------

bool sensorsBegin() {
  // SDA first. The signature is Wire.begin(sda, scl) and I have transposed it
  // before, which produces a bus that scans as completely empty and looks for
  // all the world like a soldering fault.
  Wire.begin(PIN_SDA, PIN_SCL, I2C_CLOCK_HZ);
  busStarted = true;

  bmePresent = bme.begin(I2C_ADDR_BME280, &Wire);
  if (bmePresent) {
    // Forced mode is the whole reason this sensor suits a battery node: it takes
    // one measurement when asked and then parks itself at about 0.1 uA. In
    // normal mode it would keep sampling in the background all the way through
    // deep sleep and quietly wreck the current budget.
    //
    // x1 oversampling on all three. This is a plant pot, not a barometer, and
    // heavier oversampling only lengthens the conversion I have to stay awake
    // for. The IIR filter is off for the same reason — it needs a run of
    // samples to converge, and I take exactly one every half hour.
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,   // temperature
                    Adafruit_BME280::SAMPLING_X1,   // pressure
                    Adafruit_BME280::SAMPLING_X1,   // humidity
                    Adafruit_BME280::FILTER_OFF);
  } else {
    LOGLN("bme280: no answer at 0x76 - check SDO is at GND and CSB at 3V3");
  }

  vemlPresent = veml.begin(&Wire);
  if (vemlPresent) {
    // Gain and integration time only matter here as a starting point, because
    // the automatic mode below re-picks both to suit the actual light level.
    // Left as-is this would saturate on a bright windowsill and read zero in a
    // dim room, which between them covers most of where this board will live.
    veml.setGain(VEML7700_GAIN_1);
    veml.setIntegrationTime(VEML7700_IT_100MS);
  } else {
    LOGLN("veml7700: no answer at 0x10");
  }

  return bmePresent || vemlPresent;
}

// ---------------------------------------------------------------------------

void soilProbePowerOn() {
  pinMode(PIN_SENS_PWR_EN, OUTPUT);
  digitalWrite(PIN_SENS_PWR_EN, PROBE_ON);
  probeOnAtMs = millis();
}

void soilProbePowerOff() {
  // Driving it high would work, but releasing the pin is better: R10's 100k
  // pull-up holds Q1's gate at 3V3 with no drive current at all, and it goes on
  // holding it through deep sleep and through the next boot, before this code
  // has had a chance to run. That is what R10 is for.
  digitalWrite(PIN_SENS_PWR_EN, PROBE_OFF);
  pinMode(PIN_SENS_PWR_EN, INPUT);
  probeOnAtMs = 0;
}

// ---------------------------------------------------------------------------

void sensorsReadClimateAndLight(Readings &r) {
  if (bmePresent) {
    // Ask for the one measurement, then read it out. This blocks for a few
    // milliseconds at x1 oversampling.
    if (bme.takeForcedMeasurement()) {
      r.temperatureC = bme.readTemperature();
      r.humidityPct  = bme.readHumidity();
      r.pressureHpa  = bme.readPressure() / 100.0f;

      // A dead BME280 on a live bus reads a plausible-looking 0 C / 0 %, so I
      // check rather than trust. -40 C is the part's own lower limit.
      r.climateValid = !isnan(r.temperatureC) && r.temperatureC > -40.0f &&
                       !isnan(r.humidityPct)  && r.humidityPct >= 0.0f;
    }
  }

  if (vemlPresent) {
    // Automatic mode walks gain and integration time until the raw count sits
    // in a sensible part of the range, so one call covers a dark room and a
    // south-facing sill without me choosing a compromise between them.
    float lux = veml.readLux(VEML_LUX_AUTO);
    if (!isnan(lux) && lux >= 0.0f) {
      r.lux        = lux;
      r.lightValid = true;
    }
  }
}

// ---------------------------------------------------------------------------

void sensorsReadSoil(Readings &r) {
  if (probeOnAtMs == 0) return;   // nobody switched the probe on

  // Whatever the I2C reads did not use up of the settle time, spend it here.
  // The probe is a slow analogue thing on the end of a metre of cable and it
  // needs the full quarter-second before its output means anything.
  uint32_t elapsed = millis() - probeOnAtMs;
  if (elapsed < SOIL_SETTLE_MS) delay(SOIL_SETTLE_MS - elapsed);

  r.soilRaw = adcAverageRaw(PIN_ADC_SOIL, ADC_SAMPLES);

  // Output falls as moisture rises, so dry is the larger raw number and the
  // subtraction runs the way it does on purpose.
  float span = (float)(SOIL_RAW_DRY - SOIL_RAW_WET);
  float pct  = ((float)(SOIL_RAW_DRY - r.soilRaw) / span) * 100.0f;

  // I clamp rather than reject. A probe pushed into wetter soil than the water
  // glass I calibrated in genuinely does read below my wet constant, and that
  // is 100 %, not an error. What it is not is 104 %.
  if (pct < 0.0f)   pct = 0.0f;
  if (pct > 100.0f) pct = 100.0f;

  r.soilPct = pct;

  // A raw value pinned at either rail is not soil, it is a fault: 0 or nearly 0
  // means the probe is unpowered or its signal line is shorted, and the top of
  // the range means nothing is connected and R11 is the only thing on the pin.
  r.soilValid = (r.soilRaw > 100) && (r.soilRaw < 4000);
  if (!r.soilValid) {
    LOGF("soil: raw %d is out of range - check J2 and the probe cable\n", r.soilRaw);
  }
}

// ---------------------------------------------------------------------------

void sensorsSleep() {
  // The BME280 parked itself the moment its forced measurement finished. The
  // VEML7700 sits at about 45 uA until it is told otherwise, and my whole sleep
  // budget is 210 uA, so this line is worth a fifth of it.
  if (vemlPresent) veml.enable(false);

  // Only if it was ever started. A cycle that bailed out at the 3.0 V floor
  // never touched the bus, and tearing down something that was never brought up
  // is how a low-battery wake turns into a crash.
  if (busStarted) {
    Wire.end();
    busStarted = false;
  }
}
