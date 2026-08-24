// sensors.h — the three measurements this board exists to take.
//
// Everything here is allowed to fail. A sensor that does not answer costs me one
// field in one message; it must never cost me the wake cycle, and it must never
// leave the probe powered.

#pragma once

#include <Arduino.h>

struct Readings {
  // Soil. Raw counts are what I calibrated against, so I publish both the raw
  // number and the percentage — the raw one is the number I can compare against
  // the bring-up record sheet when a reading looks wrong.
  int   soilRaw      = -1;
  float soilPct      = NAN;
  bool  soilValid    = false;

  float temperatureC = NAN;
  float humidityPct  = NAN;
  float pressureHpa  = NAN;
  bool  climateValid = false;

  float lux          = NAN;
  bool  lightValid   = false;
};

// Brings up the bus on IO38/IO39 and probes for both sensors. Returns true only
// if at least one of them answered — if neither did, there is nothing worth
// transmitting and the caller can go straight back to sleep.
bool sensorsBegin();

// Pulls SENS_PWR_EN low and starts the settle clock.
void soilProbePowerOn();

// Puts the probe back to sleep by releasing the pin to R10. Safe to call even if
// the probe was never switched on.
void soilProbePowerOff();

// Reads the two I2C sensors. Call this during the probe's settle window — it
// takes long enough to be worth doing there rather than sitting in a delay().
void sensorsReadClimateAndLight(Readings &r);

// Waits out whatever is left of the settle time, then reads the probe.
void sensorsReadSoil(Readings &r);

// Shuts the light sensor down. The BME280 puts itself back to sleep after every
// forced measurement; the VEML7700 does not, and its 45 uA would be a fifth of
// my whole sleep budget if I left it running.
void sensorsSleep();
