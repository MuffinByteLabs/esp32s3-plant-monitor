// power.h — the battery rules, and the only route to deep sleep.
//
// This file owns the half-volt that nothing else on the board is watching. The
// pack's protection module handles disasters at 2.5 V; PKCell's discharge floor
// is 3.0 V; between the two the cell is being quietly ruined and no hardware
// cares. See docs/Engineering_Notes.md section 4.

#pragma once

#include <Arduino.h>

struct BatteryState {
  uint32_t milliVolts = 0;
  float    percent    = NAN;

  // True when the cell has enough margin for the regulator to hold 3.3 V
  // through a 355 mA transmit peak.
  bool txAllowed = false;

  // True below the 3.0 V floor: read nothing, send nothing, sleep long.
  bool critical = false;

  // True when a USB host has the port open. This is a proxy, not a measurement
  // — see powerUsbHostAttached().
  bool usbHostAttached = false;

  // False when the reading cannot be believed as a state of charge, which is
  // the case whenever USB is supplying the board: the charger holds VBAT at its
  // float voltage and BAT_SENSE reports a full cell whether or not one is
  // fitted.
  bool trustworthy = false;
};

BatteryState powerReadBattery();

// Whether a USB host currently has the CDC port open.
//
// Rev A has no VBUS sense line — I never brought +5V_PROT to a GPIO — so this
// is the closest thing to a USB-present signal the board can give me. It is not
// the same question: a phone charger supplies VBUS and never opens the port, so
// this reads false while the board is plainly on USB. It is right in the case I
// actually care about, which is deciding whether a battery percentage is worth
// publishing while I am sat at the bench with the thing plugged in. A divider
// from +5V_PROT to a spare pin fixes it properly in Rev B.
bool powerUsbHostAttached();

// The only way out of a wake cycle. Parks the probe, drops the radio, and does
// not return.
void powerDeepSleep(uint32_t minutes);

// Reset reason as a short string, for the boot line. Bring-up step 13 has me
// watching this across a 24-hour soak for resets I did not ask for.
const char *powerResetReasonName();
