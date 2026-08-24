// config.h — every number this board imposes on the firmware, in one place.
//
// Most of these are not arbitrary. Each one falls out of a measurement I took or
// a limit in a datasheet, and both are written up in docs/. Where that is true I
// have named the document, so when I come back to this in a year I can re-read
// the argument instead of guessing whether a constant is safe to move.
//
// Pin numbers are GPIO numbers, and they match docs/PinMap_CheatSheet.md. If the
// two ever disagree, the cheat sheet is right — it was re-extracted from the
// board netlist after the I2C re-pin on 2026-08-15.

#pragma once

// ---------------------------------------------------------------------------
// Identity
// ---------------------------------------------------------------------------

// Goes into the MQTT topics and the Home Assistant unique_ids. Change it per
// board if I ever build more than one, otherwise two boards will fight over the
// same entities.
#define NODE_ID          "plant-01"
#define NODE_NAME        "Plant Monitor"
#define NODE_MODEL       "ESP32-S3 Plant Monitor Rev A"
#define NODE_MANUFACTURER "MuffinByte Labs"
#define FIRMWARE_VERSION "1.0.0"

// ---------------------------------------------------------------------------
// Pins — see docs/PinMap_CheatSheet.md
// ---------------------------------------------------------------------------

// I2C moved from IO4/IO5 to IO38/IO39 during layout so the bus could leave the
// module on the side that faces the sensors. Wire.begin() takes SDA first, and I
// have got that the wrong way round before, so: 38 is data, 39 is clock.
#define PIN_SDA           38
#define PIN_SCL           39

// Both analogue inputs sit on ADC1, and that matters: ADC2 is unusable on the
// ESP32-S3 while the radio is up. Nothing here has to be re-ordered around Wi-Fi
// because of it, but I still read both ADCs before the radio starts.
#define PIN_ADC_SOIL       1   // ADC1_CH0, direct from the probe, no divider
#define PIN_BAT_SENSE      2   // ADC1_CH1, VBAT / 2 through R14/R15

// Q1 is a high-side P-FET with a 100k pull-up (R10) on its gate. LOW turns the
// probe on; anything else — driven high, or the pin released entirely — leaves
// R10 holding it off. That pull-up is what keeps the probe dark through deep
// sleep and through the boot window before this code runs.
#define PIN_SENS_PWR_EN   21
#define PROBE_ON          LOW
#define PROBE_OFF         HIGH

// ---------------------------------------------------------------------------
// I2C addresses
// ---------------------------------------------------------------------------

// BME280 is at 0x76 because I tied SDO to GND (CSB to 3V3). VEML7700's address
// is fixed in silicon.
#define I2C_ADDR_BME280   0x76
#define I2C_ADDR_VEML7700 0x10
#define I2C_CLOCK_HZ      100000UL   // 100 kHz. Nothing here is in a hurry and
                                     // the probe cable is a metre of antenna.

// ---------------------------------------------------------------------------
// Battery thresholds — the half-volt that belongs to firmware
// ---------------------------------------------------------------------------
//
// The pack's own protection module (S-8261AAJMD) does not disconnect until
// 2.5 V, but PKCell's discharge floor for the cell is 3.0 V. Everything in that
// gap is wear, not disaster, and nothing on this board is watching it except
// this file. See docs/Engineering_Notes.md section 4.

// Stop transmitting below this. This is a regulator rule, not a battery rule:
// the AP2112K needs roughly 200 mV of headroom to hold 3.3 V through a 355 mA
// Wi-Fi TX peak. Waking, reading sensors and going back to sleep are all still
// fine below it — only the radio is banned.
#define TX_CUTOFF_MV      3500

// Stop everything below this. The battery rule. Below here I read nothing and
// send nothing, I just go back to sleep on the long interval and wait to be
// charged.
#define SLEEP_CUTOFF_MV   3000

// A little hysteresis so a board sitting exactly on a threshold does not flap
// between "transmit" and "do not transmit" on consecutive wakes.
#define CUTOFF_HYSTERESIS_MV 50

// ---------------------------------------------------------------------------
// Battery sense chain
// ---------------------------------------------------------------------------

// R14/R15 are 100k 1% each, so BAT_SENSE is exactly half of VBAT and reads
// 2.10 V at a full 4.20 V cell — comfortably inside the ADC's usable range at
// 12 dB attenuation.
#define BAT_DIVIDER_RATIO 2.0f

// Correction factor for the real board. Bring-up step C-8 has me comparing the
// firmware's reading against a meter on C16; whatever ratio that gives me goes
// here. 1.0 means I have not measured it yet.
#define BAT_TRIM          1.000f

// ---------------------------------------------------------------------------
// Soil probe
// ---------------------------------------------------------------------------

// The probe is a slow analogue thing on the end of a cable and it needs time to
// settle after its supply comes up. I budgeted 200 ms in the design; I use 250
// here because the extra 50 ms costs nothing at a 30-minute wake interval and
// buys margin on a cold, damp morning.
#define SOIL_SETTLE_MS    250

// Breadboard calibration through the as-built input chain: no divider, 100 nF
// filter, 12 dB attenuation, 12-bit raw. Output falls as moisture rises, so dry
// is the larger number. docs/Engineering_Notes.md section 1.
//
// These are breadboard values. Bring-up step D-11 is where I record the real
// ones on the PCB, and until I have done that I do not trust the absolute
// percentage — only the direction it moves.
#define SOIL_RAW_DRY      2900
#define SOIL_RAW_WET      1465

// The bring-up troubleshooting table says to average 8-16 samples if the ADC
// looks noisy. I average 16 from the start; it costs microseconds.
#define ADC_SAMPLES       16

// ---------------------------------------------------------------------------
// Sleep and timing
// ---------------------------------------------------------------------------

// Soil moisture does not move fast. Half an hour is plenty, and at roughly
// 210 uA of sleep floor the wake cycles are not what drains this cell anyway.
#define WAKE_INTERVAL_MIN         30

// Below SLEEP_CUTOFF_MV I stretch the interval right out. I do not stop waking
// altogether, because then a cell I put on the charger could never tell me it
// had recovered — it would just sit there flat until I pressed reset.
#define WAKE_INTERVAL_CRITICAL_MIN 360

// Hard ceiling on how long one wake cycle may stay awake. If anything hangs —
// a broker that never answers, an AP that takes the association and then goes
// quiet — I would rather lose one reading than sit here at 80 mA until the cell
// is flat. Nothing gets to overrun this.
#define AWAKE_DEADLINE_MS         30000

#define WIFI_TIMEOUT_MS           15000
#define MQTT_TIMEOUT_MS            5000

// ---------------------------------------------------------------------------
// MQTT
// ---------------------------------------------------------------------------

// One retained state topic carrying a small JSON object. Home Assistant's
// discovery entries all point at this same topic and pick their field out of it
// with a value_template, which means one publish per wake instead of eight.
#define MQTT_BASE_TOPIC   "muffinbyte/plantmonitor"
#define HA_DISCOVERY_PREFIX "homeassistant"

// Discovery payloads are far bigger than PubSubClient's 256-byte default buffer,
// and when they overflow the library fails silently — publish() just returns
// false and nothing appears in Home Assistant. This cost me an evening once.
#define MQTT_BUFFER_BYTES 1024

// Re-announce discovery every this many wakes as well as on a cold boot. The
// configs are retained so in theory once is enough, but a broker rebuilt from
// an empty database would otherwise leave the entities orphaned until I next
// pulled the power.
#define DISCOVERY_REPUBLISH_EVERY 48

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

// Serial here is the native USB CDC — there is no bridge chip on this board, so
// this only produces anything when a host is actually attached. Set to 0 for a
// deployed board; the wait-for-host below is the only part that costs real time.
#define DEBUG_SERIAL      1
#define SERIAL_BAUD       115200

// How long to wait at boot for a USB host to open the port before giving up and
// getting on with it. Only spent when a host is physically plugged in.
#define SERIAL_WAIT_MS    1500

// ---------------------------------------------------------------------------
// Optional static IP
// ---------------------------------------------------------------------------
//
// DHCP costs somewhere between a few hundred milliseconds and a couple of
// seconds of full-power radio time on every single wake, which over a hundred
// days is a real slice of the cell. A static address skips the whole exchange.
// Off by default because it needs an address my router will not hand out to
// something else, and a board that cannot get on the network is worse than a
// board that takes an extra second.
#define USE_STATIC_IP     0
#define STATIC_IP         "192.168.1.50"
#define STATIC_GATEWAY    "192.168.1.1"
#define STATIC_SUBNET     "255.255.255.0"
#define STATIC_DNS        "192.168.1.1"
