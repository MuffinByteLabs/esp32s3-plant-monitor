// net.h — Wi-Fi and MQTT, on a strict clock.
//
// Everything in here is the expensive part of a wake cycle. The radio is the
// only thing on this board that draws current in the hundreds of milliamps, and
// it is the only reason the 3.5 V transmit cutoff exists. So every call here
// either finishes quickly or gives up: nothing is allowed to wait indefinitely
// on a network that is not coming back.

#pragma once

#include <Arduino.h>
#include "power.h"
#include "sensors.h"

// Associates, using the cached channel and BSSID from the last successful wake
// if there is one. Returns false on timeout, having left the radio off.
bool netConnectWifi();

// Connects to the broker and publishes Home Assistant discovery if this is a
// cold boot or the republish interval has come round.
bool netConnectBroker(uint32_t bootCount);

// One retained JSON message with everything this wake measured.
bool netPublishState(const Readings &r, const BatteryState &b, uint32_t bootCount);

// Closes the session politely so the broker does not have to time it out.
void netShutdown();

int8_t netRssi();
