#include "net.h"

#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "log.h"

#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #error "No secrets.h. Copy secrets.h.example to secrets.h and fill in your Wi-Fi and broker details - secrets.h is gitignored on purpose."
#endif

static WiFiClient   wifiClient;
static PubSubClient mqtt(wifiClient);

// Cached across deep sleep. A cold association has to scan every channel for the
// SSID before it can do anything else; handed the channel and the BSSID it went
// to last time, it skips straight to the association and saves something like a
// second and a half of radio at full power. If the AP has moved channel the
// attempt fails and the retry below does it the slow way.
RTC_DATA_ATTR static uint8_t rtcBssid[6]  = {0};
RTC_DATA_ATTR static uint8_t rtcChannel   = 0;
RTC_DATA_ATTR static bool    rtcHaveAp    = false;

static char stateTopic[96];

static void buildTopics() {
  snprintf(stateTopic, sizeof(stateTopic), "%s/%s/state", MQTT_BASE_TOPIC, NODE_ID);
}

// ---------------------------------------------------------------------------

static bool waitForConnection(uint32_t timeoutMs) {
  uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - started > timeoutMs) return false;
    delay(50);
  }
  return true;
}

bool netConnectWifi() {
  // Off by default, so nothing writes the SSID and password back to NVS on every
  // single association. Left on, that is a flash write every half hour for the
  // life of the board, to store something the firmware already knows.
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);

#if USE_STATIC_IP
  IPAddress ip, gw, mask, dns;
  if (ip.fromString(STATIC_IP) && gw.fromString(STATIC_GATEWAY) &&
      mask.fromString(STATIC_SUBNET) && dns.fromString(STATIC_DNS)) {
    WiFi.config(ip, gw, mask, dns);
  } else {
    LOGLN("wifi: static IP strings did not parse - falling back to DHCP");
  }
#endif

  bool connected = false;

  if (rtcHaveAp) {
    LOGF("wifi: fast path, ch %u\n", (unsigned)rtcChannel);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, rtcChannel, rtcBssid);
    connected = waitForConnection(WIFI_TIMEOUT_MS / 2);
    if (!connected) {
      // The AP moved, or it is a different one now. Forget what I thought I
      // knew and do it properly.
      LOGLN("wifi: fast path failed - rescanning");
      rtcHaveAp = false;
      WiFi.disconnect(true);
      delay(100);
    }
  }

  if (!connected) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    connected = waitForConnection(WIFI_TIMEOUT_MS);
  }

  if (!connected) {
    LOGLN("wifi: no association - giving up on this cycle");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return false;
  }

  memcpy(rtcBssid, WiFi.BSSID(), 6);
  rtcChannel = WiFi.channel();
  rtcHaveAp  = true;

  LOGF("wifi: %s, %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  return true;
}

int8_t netRssi() {
  return (WiFi.status() == WL_CONNECTED) ? (int8_t)WiFi.RSSI() : 0;
}

// ---------------------------------------------------------------------------
// Home Assistant discovery
// ---------------------------------------------------------------------------
//
// Every entity below points at the same retained state topic and picks its own
// field out of the JSON with a value_template. That is one publish per wake
// instead of nine, which on a battery node is the difference between a short
// burst of radio and a long one.

struct SensorSpec {
  const char *key;         // the field name in the state JSON, and the object_id
  const char *name;        // what it is called in Home Assistant
  const char *deviceClass; // or nullptr
  const char *unit;        // or nullptr
  const char *icon;        // or nullptr
  bool        diagnostic;  // tucks it away in the device's diagnostics section
};

static const SensorSpec kSensors[] = {
  { "soil_pct",     "Soil moisture", nullptr,           "%",    "mdi:water-percent", false },
  { "temp_c",       "Temperature",   "temperature",     "°C", nullptr,          false },
  { "humidity_pct", "Humidity",      "humidity",        "%",    nullptr,             false },
  { "pressure_hpa", "Pressure",      "pressure",        "hPa",  nullptr,             false },
  { "lux",          "Illuminance",   "illuminance",     "lx",   nullptr,             false },
  { "battery_pct",  "Battery",       "battery",         "%",    nullptr,             true  },
  { "battery_v",    "Battery voltage", "voltage",       "V",    nullptr,             true  },
  { "soil_raw",     "Soil raw",      nullptr,           nullptr, "mdi:counter",      true  },
  { "rssi",         "Wi-Fi signal",  "signal_strength", "dBm",  nullptr,             true  },
};

static void publishDiscovery() {
  static char topic[160];
  static char payload[MQTT_BUFFER_BYTES];

  for (const SensorSpec &s : kSensors) {
    snprintf(topic, sizeof(topic), "%s/sensor/%s/%s/config",
             HA_DISCOVERY_PREFIX, NODE_ID, s.key);

    int n = snprintf(payload, sizeof(payload),
      "{"
        "\"name\":\"%s\","
        "\"uniq_id\":\"%s_%s\","
        "\"stat_t\":\"%s\","
        "\"val_tpl\":\"{{ value_json.%s }}\","
        "\"stat_cla\":\"measurement\","
        // If nothing arrives in two and a half wake intervals I want the entity
        // to say so rather than sit there showing a reading from yesterday. This
        // is how a sleeping node stays honest without a last-will message, which
        // would mark it offline every time it went to sleep - which is always.
        "\"exp_aft\":%u,"
        "%s%s%s"      // device_class
        "%s%s%s"      // unit
        "%s%s%s"      // icon
        "%s"          // entity_category
        "\"dev\":{"
          "\"ids\":[\"%s\"],"
          "\"name\":\"%s\","
          "\"mdl\":\"%s\","
          "\"mf\":\"%s\","
          "\"sw\":\"%s\""
        "}"
      "}",
      s.name, NODE_ID, s.key, stateTopic, s.key,
      (unsigned)(WAKE_INTERVAL_MIN * 60 * 5 / 2),
      s.deviceClass ? "\"dev_cla\":\"" : "", s.deviceClass ? s.deviceClass : "", s.deviceClass ? "\"," : "",
      s.unit        ? "\"unit_of_meas\":\"" : "", s.unit ? s.unit : "", s.unit ? "\"," : "",
      s.icon        ? "\"ic\":\"" : "", s.icon ? s.icon : "", s.icon ? "\"," : "",
      s.diagnostic  ? "\"ent_cat\":\"diagnostic\"," : "",
      NODE_ID, NODE_NAME, NODE_MODEL, NODE_MANUFACTURER, FIRMWARE_VERSION);

    if (n < 0 || n >= (int)sizeof(payload)) {
      LOGF("discovery: payload for %s did not fit - skipped\n", s.key);
      continue;
    }

    // Retained, so Home Assistant rebuilds the whole device from the broker
    // after a restart without waiting for the board to wake up.
    if (!mqtt.publish(topic, payload, true)) {
      LOGF("discovery: publish failed for %s\n", s.key);
    }
    mqtt.loop();
  }

  LOGLN("discovery: published");
}

// ---------------------------------------------------------------------------

bool netConnectBroker(uint32_t bootCount) {
  buildTopics();

  mqtt.setServer(MQTT_HOST, MQTT_PORT);

  // PubSubClient's buffer is 256 bytes by default and the discovery payloads are
  // twice that. When they overflow, publish() just returns false and nothing
  // appears in Home Assistant - no error, no warning, nothing on the wire.
  mqtt.setBufferSize(MQTT_BUFFER_BYTES);
  mqtt.setSocketTimeout(MQTT_TIMEOUT_MS / 1000);

  bool ok;
  if (strlen(MQTT_USER) > 0) {
    ok = mqtt.connect(NODE_ID, MQTT_USER, MQTT_PASSWORD);
  } else {
    ok = mqtt.connect(NODE_ID);
  }

  if (!ok) {
    LOGF("mqtt: connect failed, state %d\n", mqtt.state());
    return false;
  }

  // Cold boot means Home Assistant may never have heard of this board, and the
  // periodic republish covers the case where the broker was rebuilt from empty
  // and quietly dropped the retained configs.
  if (bootCount <= 1 || (bootCount % DISCOVERY_REPUBLISH_EVERY) == 0) {
    publishDiscovery();
  }

  return true;
}

// ---------------------------------------------------------------------------

// Appends "key":value, but only when the reading is real. A field I leave out
// makes its value_template render empty, and Home Assistant then ignores that
// entity for this message and keeps whatever it had. That is exactly what I
// want from a sensor that failed to answer: no update, rather than a zero that
// looks like a measurement.
static int appendFloat(char *buf, size_t cap, int pos, bool valid,
                       const char *key, float value, uint8_t decimals) {
  if (!valid || isnan(value)) return pos;
  if (pos < 1 || (size_t)pos >= cap) return pos;   // no room left, say nothing
  return pos + snprintf(buf + pos, cap - pos, "%s\"%s\":%.*f",
                        pos > 1 ? "," : "", key, decimals, value);
}

static int appendInt(char *buf, size_t cap, int pos, bool valid,
                     const char *key, long value) {
  if (!valid) return pos;
  if (pos < 1 || (size_t)pos >= cap) return pos;
  return pos + snprintf(buf + pos, cap - pos, "%s\"%s\":%ld",
                        pos > 1 ? "," : "", key, value);
}

bool netPublishState(const Readings &r, const BatteryState &b, uint32_t bootCount) {
  static char payload[MQTT_BUFFER_BYTES];

  int p = snprintf(payload, sizeof(payload), "{");

  p = appendFloat(payload, sizeof(payload), p, r.soilValid,    "soil_pct",     r.soilPct,       1);
  p = appendInt  (payload, sizeof(payload), p, r.soilValid,    "soil_raw",     r.soilRaw);
  p = appendFloat(payload, sizeof(payload), p, r.climateValid, "temp_c",       r.temperatureC,  2);
  p = appendFloat(payload, sizeof(payload), p, r.climateValid, "humidity_pct", r.humidityPct,   1);
  p = appendFloat(payload, sizeof(payload), p, r.climateValid, "pressure_hpa", r.pressureHpa,   1);
  p = appendFloat(payload, sizeof(payload), p, r.lightValid,   "lux",          r.lux,           1);

  // Battery only when the number means something. With USB supplying the board
  // the charger holds VBAT at its float voltage, so this would read a full cell
  // whether or not one is even fitted - so I say nothing at all rather than
  // publish a comfortable lie.
  p = appendFloat(payload, sizeof(payload), p, b.trustworthy, "battery_pct", b.percent, 0);
  p = appendFloat(payload, sizeof(payload), p, b.trustworthy, "battery_v",
                  (float)b.milliVolts / 1000.0f, 3);

  p = appendInt(payload, sizeof(payload), p, true, "rssi", (long)netRssi());
  p = appendInt(payload, sizeof(payload), p, true, "boot", (long)bootCount);

  if (p < 0 || p >= (int)sizeof(payload) - 2) {
    LOGLN("state: payload overflowed - not published");
    return false;
  }
  snprintf(payload + p, sizeof(payload) - p, "}");

  bool ok = mqtt.publish(stateTopic, payload, true);
  LOGF("state: %s %s\n", ok ? "published" : "FAILED", payload);
  mqtt.loop();
  return ok;
}

// ---------------------------------------------------------------------------

void netShutdown() {
  if (mqtt.connected()) {
    // A clean DISCONNECT rather than just dropping the socket. Costs a few
    // milliseconds and saves the broker holding a dead session open until its
    // own keepalive gives up on me.
    mqtt.disconnect();
  }
  wifiClient.stop();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}
