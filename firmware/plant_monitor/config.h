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

// Correction factor for the real board. Bring-up step C-8: compare the
// firmware's reading against a meter on VBAT and put the ratio here.
//
// MEASURED 2026-09-09, board #1: the meter and the firmware agreed to the
// meter's own resolution, so the ratio is 1.000 and stays 1.000. That is not
// "unmeasured" any more - it is a real result, and a good one: the ESP32-S3's
// factory eFuse ADC calibration plus two 1 % resistors landed inside about
// 0.3 % without any help. The residual is bounded by the meter, not the board,
// so a better trim would need a better meter rather than more care.
#define BAT_TRIM          1.000f

// ---------------------------------------------------------------------------
// Soil probe
// ---------------------------------------------------------------------------

// The probe is a slow analogue thing on the end of a cable and it needs time to
// settle after its supply comes up. I budgeted 200 ms in the design; I use 250
// here because the extra 50 ms costs nothing at a 30-minute wake interval and
// buys margin on a cold, damp morning.
#define SOIL_SETTLE_MS    250

// Recorded on board #1 through this probe and this cable, 2026-09-13. This is
// bring-up step D-11, finally done. Output falls as moisture rises, so dry is
// the larger number. docs/Engineering_Notes.md section 1.
//
//   2267   probe in open air              ->   0 %
//   2011   soil that wants watering       ->  25 %   (ALERT_SOIL_DRY_PCT)
//   1229   soil immediately after a soak  -> 100 %
//
// These replace the breadboard values this shipped with (2900 / 1465), which
// were measured through the same input chain but not through this probe on this
// board — and they were wrong by enough to matter. With 2900 as dry, the probe
// lying in open air read 45.7 %, and since air is as dry as anything ever gets,
// 45.7 % was the floor. The percentage could not physically go below it, which
// made a dry-soil alert at 30 % unreachable no matter how parched the pot got.
// The scale was not merely inaccurate, it was missing its bottom half.
#define SOIL_RAW_DRY      2267
#define SOIL_RAW_WET      1229

// The bring-up troubleshooting table says to average 8-16 samples if the ADC
// looks noisy. I average 16 from the start; it costs microseconds.
#define ADC_SAMPLES       16

// ---------------------------------------------------------------------------
// Sleep and timing
// ---------------------------------------------------------------------------

// TEST MODE. Set to 0 before the board goes anywhere near a plant.
// A bench test where each attempt costs half an hour is a bench test nobody
// finishes, so this shortens the wake interval to one minute. It changes
// nothing else - every threshold, cutoff and timeout stays exactly as it is.
#define TEST_MODE                 0

// Soil moisture does not move fast. Half an hour is plenty, and at roughly
// 210 uA of sleep floor the wake cycles are not what drains this cell anyway.
#if TEST_MODE
  #define WAKE_INTERVAL_MIN       1
#else
  #define WAKE_INTERVAL_MIN       30
#endif

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
// Transmit backends - pick one or both
// ---------------------------------------------------------------------------
//
// Two independent ways for a wake cycle to report what it measured. They share
// the Wi-Fi association, so running both costs one extra round trip, not two.
//
//   USE_MQTT  - the original path. Publishes one retained JSON message and the
//               Home Assistant discovery configs, so entities appear on their
//               own. Needs a broker on the network. Leave this at 0 until one
//               exists, or every wake wastes MQTT_TIMEOUT_MS failing to reach it.
//
//   USE_NTFY  - an HTTPS POST to ntfy.sh, which turns into a push notification
//               on a phone. No broker, no account, no server: the topic name is
//               the only credential, which is why it has to be unguessable.
//               Good for "is this thing alive", useless as a time series -
//               ntfy.sh holds messages in memory for twelve hours and then they
//               are gone. See the alerts block below for when it is allowed to
//               interrupt you now that something else keeps the history.
//
//   USE_THINGSPEAK
//             - a form POST to api.thingspeak.com. This is the time series and
//               the graphs: it stores every reading and serves them at a URL
//               that needs no login to read, so the history outlives both the
//               board and my laptop.
//
// Setting all three to 0 is legal - the board reads and sleeps and tells nobody.

#define USE_MQTT          0
#define USE_NTFY          1
#define USE_THINGSPEAK    1

// ---------------------------------------------------------------------------
// ntfy
// ---------------------------------------------------------------------------

// NTFY_TOPIC lives in secrets.h with the Wi-Fi credentials, because anyone who
// knows it can read these readings and can push notifications to this phone.
// It is a password that happens to look like a URL path.
#define NTFY_HOST         "ntfy.sh"
#define NTFY_PORT         443

// TLS costs roughly 1-2 s of full-power radio per wake for the handshake -
// call it 15 % of this board's daily budget at a 30 minute interval. I am
// paying it because the alternative is plaintext across whatever network this
// board is on. Self-hosting ntfy on the LAN would let this drop to plain HTTP.
//
// Certificates are not verified (setInsecure). Pinning a root CA into a board
// that may sleep for months means a firmware update the day that CA rotates,
// and the payload here is soil moisture. Worth knowing, not worth fixing.
#define NTFY_TIMEOUT_MS   8000

// 1 = min, 2 = low (no sound), 3 = default, 4 = high, 5 = max.
//
// Two of each: the routine one is deliberately quiet, because with
// NTFY_ALERTS_ONLY at 1 the only routine message left is the all-clear and that
// does not need to wake anybody. The alert one is allowed to make noise,
// because the whole point of the split is that when this thing does buzz it
// means something.
#define NTFY_PRIORITY       "2"
#define NTFY_PRIORITY_ALERT "4"

// Shown as the notification's emoji. See docs.ntfy.sh/emojis
#define NTFY_TAGS           "potted_plant"
#define NTFY_TAGS_ALERT     "warning,potted_plant"

// ---------------------------------------------------------------------------
// ThingSpeak
// ---------------------------------------------------------------------------
//
// TS_CHANNEL_ID and TS_WRITE_API_KEY live in secrets.h. The write key is the
// only real credential here: the channel itself is public on purpose, so anyone
// with the URL can read the graphs and nobody without the key can add to them.
//
// Eight fields is the hard ceiling on a channel and this board produces ten
// numbers, so battery_pct and the boot counter travel in the status string
// instead. Nothing is actually lost - battery_pct is a pure function of
// battery_v through the discharge curve in power.cpp.
//
//   field1  soil_pct        field5  lux
//   field2  temp_c          field6  battery_v
//   field3  humidity_pct    field7  rssi
//   field4  pressure_hpa    field8  soil_raw
//
// Name the fields in exactly that order when creating the channel. Nothing
// checks it and nothing will warn you: mislabel field3 and the graph will
// cheerfully plot humidity under the word "pressure" forever.
#define TS_HOST           "api.thingspeak.com"

// The free tier accepts one update per channel every 15 s. WAKE_INTERVAL_MIN is
// a minute even in TEST_MODE, so there is nothing here to trip over - but the
// number is written down because a future me shortening the wake interval to
// "just watch it for a second" would otherwise get silent rejections.
#define TS_MIN_INTERVAL_S 15

// 0 = plain HTTP, 1 = HTTPS.
//
// These are soil readings on a deliberately public channel, and a TLS handshake
// is 1-2 s of radio at full power on every single wake - the same cost the rest
// of this firmware bends over backwards to avoid. Plain HTTP hands it back. The
// write key does travel in clear, and the worst anyone on the path can do with
// it is write junk readings into a channel of houseplant data. Set this to 1 if
// that trade stops being worth it.
#define TS_USE_TLS        0
#define TS_TIMEOUT_MS     8000

// ---------------------------------------------------------------------------
// Alerts - when ntfy is allowed to interrupt you
// ---------------------------------------------------------------------------
//
// Once ThingSpeak holds the history there is no reason to push a notification
// every wake. A phone that buzzes 48 times a day is a phone you stop reading,
// and by then it can no longer tell you the one thing you needed to hear.
//
// 1 = notify when something goes wrong, and again when it comes right.
// 0 = the old behaviour, a full report every wake. Useful during bring-up.
#define NTFY_ALERTS_ONLY  1

// Dry soil. 25 % is raw 2008 on the calibration above - within three counts of
// the 2011 I measured in a pot that wanted watering, so this fires just as the
// soil reaches that state rather than after it.
//
// Each condition latches: once it has fired it holds until the reading climbs
// back past the threshold by the hysteresis band, so a probe sitting exactly on
// the line does not alert, recover and alert again forever. 5 points is 52 raw
// counts here, comfortably more than the ~15 counts of drift these readings
// showed overnight, and a real watering clears it outright.
#define ALERT_SOIL_DRY_PCT     25.0f
#define ALERT_SOIL_HYST_PCT     5.0f

// Low cell. Only ever evaluated when the battery reading is trustworthy, which
// means the bench never generates one of these while a USB host is attached.
#define ALERT_BATT_LOW_PCT     20.0f
#define ALERT_BATT_HYST_PCT     5.0f

// A condition that stays true is repeated at most this often, counted in wake
// cycles rather than minutes so it scales with whatever the interval is. At the
// 30 minute production interval this is once a day.
#define ALERT_REPEAT_WAKES       48

// Whether a sensor that stopped answering deserves a notification of its own.
// Either way its readings simply stop appearing on the graph.
#define ALERT_ON_SENSOR_FAULT     1

// The conditions themselves. net.cpp puts these in the notification title and
// plant_monitor.ino decides which are true, so they have to live somewhere both
// can see - and they belong next to the thresholds they are derived from.
#define ALERT_BIT_SOIL_DRY     (1u << 0)
#define ALERT_BIT_BATT_LOW     (1u << 1)
#define ALERT_BIT_SENSOR_FAULT (1u << 2)

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
