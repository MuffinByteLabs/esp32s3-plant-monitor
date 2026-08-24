// log.h — printing that costs nothing when it is switched off.
//
// Serial on this board is the native USB CDC on IO19/IO20. There is no bridge
// chip, so if no host is plugged in there is no port to print to and every one
// of these calls goes nowhere. That is fine, but I would rather the compiler
// threw them away entirely on a deployed board than have it discover that at
// run time, every wake, forever.

#pragma once

#include <Arduino.h>
#include "config.h"

#if DEBUG_SERIAL
  #define LOG(x)    Serial.print(x)
  #define LOGLN(x)  Serial.println(x)
  #define LOGF(...) Serial.printf(__VA_ARGS__)
#else
  #define LOG(x)    do {} while (0)
  #define LOGLN(x)  do {} while (0)
  #define LOGF(...) do {} while (0)
#endif

// Starts the CDC either way, because whether a host has opened the port is the
// only USB-presence signal Rev A gives me and the battery logic wants to know.
// The wait is only ever spent when a host is actually there.
inline void logBegin() {
  Serial.begin(SERIAL_BAUD);
#if DEBUG_SERIAL
  uint32_t started = millis();
  while (!Serial && (millis() - started) < SERIAL_WAIT_MS) delay(10);
  delay(50);   // the host end needs a moment after enumeration or the first
               // line gets eaten
#endif
}
