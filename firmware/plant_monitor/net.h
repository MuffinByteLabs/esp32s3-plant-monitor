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

// One POST to ntfy.sh, which lands as a push notification on the phone
// subscribed to NTFY_TOPIC. Independent of the broker: it needs Wi-Fi and
// nothing else, so it is what proves the board is alive before any of the MQTT
// infrastructure exists. No-op unless USE_NTFY is 1.
//
// `alerts` is the ALERT_BIT_* mask for this wake and only changes the title -
// the body is the same full report either way, because the moment you are being
// told something is wrong is exactly the moment you want the other readings
// too. Pass 0 for a routine report.
bool netPublishNtfy(const Readings &r, const BatteryState &b, uint32_t bootCount,
                    uint32_t alerts);

// One form POST to ThingSpeak: eight numbered fields and a status line. This is
// the one that builds the graph, so it is the one that runs every wake, and it
// is deliberately the cheapest of the three - see TS_USE_TLS in config.h.
// No-op unless USE_THINGSPEAK is 1.
bool netPublishThingSpeak(const Readings &r, const BatteryState &b,
                          uint32_t bootCount);

// Closes the session politely so the broker does not have to time it out.
void netShutdown();

int8_t netRssi();
