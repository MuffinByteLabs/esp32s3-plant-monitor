// adc.h — the one place either analogue input gets read.
//
// Both of them go through the same averaging helper so that if I ever change my
// mind about sample counts or attenuation, there is exactly one thing to edit.

#pragma once

#include <Arduino.h>
#include "config.h"

// 12 dB attenuation gives an effective input range of about 0 to 2900 mV, which
// is what both of my chains were sized against: the soil probe reads about
// 2.20 V in air, and BAT_SENSE tops out at 2.10 V with a full cell. Anything
// less attenuation and dry soil would clip.
inline void adcConfigurePin(uint8_t pin) {
  analogSetPinAttenuation(pin, ADC_11db);
}

// Raw counts, averaged. I use this for soil, where the calibration constants I
// measured are raw counts and converting to millivolts first would only add a
// rounding step.
inline int adcAverageRaw(uint8_t pin, uint8_t samples) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  return (int)(sum / samples);
}

// Millivolts, averaged. analogReadMilliVolts() applies the per-chip calibration
// burned into the eFuses at the factory, which is worth having for the battery
// reading — I am asking the ADC to tell me the difference between 3.50 V and
// 3.45 V and deciding whether to switch the radio on based on the answer.
inline uint32_t adcAverageMilliVolts(uint8_t pin, uint8_t samples) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    sum += analogReadMilliVolts(pin);
    delayMicroseconds(200);
  }
  return sum / samples;
}
