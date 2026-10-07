#include "net.h"

#include <WiFi.h>
#include <PubSubClient.h>

// Unconditional on purpose, even though only the ntfy path uses them.
// These were briefly wrapped in "#if USE_NTFY", which does not work here: this
// is above the #include of config.h, so USE_NTFY was still undefined, the
// preprocessor read it as 0, and the headers were skipped - while the function
// further down, which IS below config.h, saw USE_NTFY as 1 and compiled without
// them. Both parts of the ESP32 core, and the linker drops whatever is unused,
// so there is nothing to gain by making them conditional and a silent ordering
// bug to lose.
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <stdarg.h>

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

// ---------------------------------------------------------------------------
// ntfy
// ---------------------------------------------------------------------------
//
// One HTTPS POST. The body is the notification text; everything else - title,
// priority, emoji - travels as headers, which is why the title below is kept
// strictly ASCII. HTTP headers are not a UTF-8 transport, and a degree sign in
// one is how you get a notification that arrives mangled or not at all.
//
// This no longer runs every wake. ThingSpeak keeps the history; this fires when
// something wants a human - see the alerts block in config.h.
//
// Every field is optional here in a way it is not in the MQTT payload: a sensor
// that failed simply does not appear, rather than publishing a null that some
// dashboard then plots as zero.

#if USE_NTFY

// snprintf returns the length it *would* have written, not the length it did.
// Adding that to a cursor walks it past the end of the buffer on the first
// truncation, and then the next call computes `cap - pos` as a negative int
// that becomes an enormous size_t - at which point snprintf is writing off the
// end of the array and the bounds check that was supposed to catch it runs
// afterwards, too late. Clamping once here is cheaper than remembering to think
// about it at eleven call sites.
static int appendTo(char *buf, size_t cap, int pos, const char *fmt, ...) {
  if (pos < 0 || (size_t)pos >= cap) return (int)cap;

  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf + pos, cap - pos, fmt, ap);
  va_end(ap);

  if (n < 0) return pos;
  pos += n;
  return ((size_t)pos > cap) ? (int)cap : pos;
}

bool netPublishNtfy(const Readings &r, const BatteryState &b, uint32_t bootCount,
                    uint32_t alerts) {
  if (WiFi.status() != WL_CONNECTED) {
    LOGLN("ntfy: no association - nothing sent");
    return false;
  }

  // --- the one-glance summary, ASCII only (this becomes a header) ---
  char title[96];
  int t = 0;

  // Whatever is wrong goes first. A notification is read left to right and on a
  // lock screen the left-hand third is often all of it that is ever read.
  if (alerts & ALERT_BIT_SOIL_DRY) {
    t = appendTo(title, sizeof(title), t, "DRY! ");
  }
  if (alerts & ALERT_BIT_BATT_LOW) {
    t = appendTo(title, sizeof(title), t, "LOW BATTERY! ");
  }
  if (alerts & ALERT_BIT_SENSOR_FAULT) {
    t = appendTo(title, sizeof(title), t, "SENSOR FAULT! ");
  }
  if (alerts == 0) {
    // Either a routine report, or the all-clear after a condition let go. Both
    // read the same on a phone and both mean the same thing: nothing to do.
    t = appendTo(title, sizeof(title), t, "OK: ");
  }

  t = appendTo(title, sizeof(title), t, "%s", NODE_NAME);

  if (r.soilValid) {
    t = appendTo(title, sizeof(title), t, " | Soil %.0f%%", r.soilPct);
  }
  if (b.trustworthy) {
    t = appendTo(title, sizeof(title), t, " | Batt %.0f%%", b.percent);
  } else {
    // Not a failure. The charger is holding VBAT up, so there is no state of
    // charge to report - see powerReadBattery().
    t = appendTo(title, sizeof(title), t, " | on USB");
  }

  // --- the detail, UTF-8 is fine here (this is the request body) ---
  char body[400];
  int n = 0;

  if (r.soilValid) {
    n = appendTo(body, sizeof(body), n, "Soil     %.1f %%  (raw %d)\n",
                 r.soilPct, r.soilRaw);
  } else {
    n = appendTo(body, sizeof(body), n, "Soil     no reading\n");
  }

  if (r.climateValid) {
    n = appendTo(body, sizeof(body), n,
                 "Temp     %.2f \xC2\xB0""C\nHumidity %.1f %%\nPressure %.1f hPa\n",
                 r.temperatureC, r.humidityPct, r.pressureHpa);
  } else {
    n = appendTo(body, sizeof(body), n, "Climate  no reading (BME280)\n");
  }

  if (r.lightValid) {
    n = appendTo(body, sizeof(body), n, "Light    %.1f lx\n", r.lux);
  } else {
    n = appendTo(body, sizeof(body), n, "Light    no reading (VEML7700)\n");
  }

  if (b.trustworthy) {
    n = appendTo(body, sizeof(body), n, "Battery  %.0f %%  (%.3f V)\n",
                 b.percent, (float)b.milliVolts / 1000.0f);
  } else {
    n = appendTo(body, sizeof(body), n,
                 "Battery  not reported - USB host attached\n");
  }

  n = appendTo(body, sizeof(body), n, "Wi-Fi    %d dBm\nWake     #%u\n",
               (int)netRssi(), (unsigned)bootCount);

  if ((size_t)n >= sizeof(body)) {
    LOGLN("ntfy: body overflowed - not sent");
    return false;
  }

  // --- send ---
  //
  // Certificates are not verified. See the note in config.h: pinning a root CA
  // into a board that sleeps for months buys a firmware update the day that CA
  // rotates, and this payload is soil moisture.
  WiFiClientSecure tls;
  tls.setInsecure();
  tls.setTimeout(NTFY_TIMEOUT_MS / 1000);

  HTTPClient http;
  char url[128];
  snprintf(url, sizeof(url), "https://%s/%s", NTFY_HOST, NTFY_TOPIC);

  if (!http.begin(tls, url)) {
    LOGLN("ntfy: begin() failed - bad URL?");
    return false;
  }

  http.setTimeout(NTFY_TIMEOUT_MS);
  http.setConnectTimeout(NTFY_TIMEOUT_MS);
  http.addHeader("Title",    title);
  http.addHeader("Priority", alerts ? NTFY_PRIORITY_ALERT : NTFY_PRIORITY);
  http.addHeader("Tags",     alerts ? NTFY_TAGS_ALERT : NTFY_TAGS);

  int code = http.POST((uint8_t *)body, (size_t)n);
  bool ok  = (code >= 200 && code < 300);

  if (ok) {
    LOGF("ntfy: sent (HTTP %d) %s\n", code, title);
  } else if (code > 0) {
    LOGF("ntfy: rejected, HTTP %d - %s\n", code, http.getString().c_str());
  } else {
    // Negative codes are HTTPClient's own: -1 connection refused, -11 timeout.
    // On a network that needs a captive-portal login this is what it looks like.
    LOGF("ntfy: no reply (%d) - DNS, firewall or captive portal?\n", code);
  }

  http.end();
  return ok;
}

#else   // !USE_NTFY

bool netPublishNtfy(const Readings &, const BatteryState &, uint32_t, uint32_t) {
  return false;
}

#endif  // USE_NTFY

// ---------------------------------------------------------------------------
// ThingSpeak
// ---------------------------------------------------------------------------
//
// A form POST of eight numbered fields. This is the one that has to happen on
// every single wake, because it is the one building the graph - a missing ntfy
// notification is a shrug, a missing ThingSpeak entry is a hole in the record.
//
// Same rule as the MQTT payload: a field whose sensor failed is left out of the
// body entirely rather than sent as zero. ThingSpeak stores the entry with that
// field blank, so the line on the chart breaks where the reading was missing
// instead of diving to the bottom of the axis and inventing a drought.

#if USE_THINGSPEAK

static int tsAppendFloat(char *buf, size_t cap, int pos, bool valid,
                         const char *key, float value, uint8_t decimals) {
  if (!valid || isnan(value)) return pos;
  if (pos < 0 || (size_t)pos >= cap) return pos;
  int n = snprintf(buf + pos, cap - pos, "&%s=%.*f", key, decimals, value);
  if (n < 0) return pos;
  pos += n;
  return ((size_t)pos > cap) ? (int)cap : pos;
}

static int tsAppendInt(char *buf, size_t cap, int pos, bool valid,
                       const char *key, long value) {
  if (!valid) return pos;
  if (pos < 0 || (size_t)pos >= cap) return pos;
  int n = snprintf(buf + pos, cap - pos, "&%s=%ld", key, value);
  if (n < 0) return pos;
  pos += n;
  return ((size_t)pos > cap) ? (int)cap : pos;
}

// The status line is free text, so unlike the numbers it has to survive form
// encoding. Only the unreserved set goes through untouched; everything else
// becomes %XX. Spaces could be '+' instead, but %20 is one less special case to
// hold in your head at two in the morning.
static int tsAppendEncoded(char *buf, size_t cap, int pos, const char *s) {
  static const char hex[] = "0123456789ABCDEF";

  // An empty string would otherwise fall straight through the loop to the
  // terminator write below, which on a cursor already sitting at cap is one
  // byte off the end of the array.
  if (pos < 0 || (size_t)pos >= cap) return (int)cap;

  for (; *s; s++) {
    unsigned char c = (unsigned char)*s;
    bool plain = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                 (c >= '0' && c <= '9') ||
                 c == '-' || c == '_' || c == '.' || c == '~';

    size_t need = plain ? 1 : 3;
    if (pos < 0 || (size_t)pos + need >= cap) return (int)cap;  // no room, stop

    if (plain) {
      buf[pos++] = (char)c;
    } else {
      buf[pos++] = '%';
      buf[pos++] = hex[c >> 4];
      buf[pos++] = hex[c & 0x0F];
    }
  }

  buf[pos] = '\0';
  return pos;
}

bool netPublishThingSpeak(const Readings &r, const BatteryState &b,
                          uint32_t bootCount) {
  if (WiFi.status() != WL_CONNECTED) {
    LOGLN("ts: no association - nothing sent");
    return false;
  }

  // On a cold boot only. The one time you want this URL is the time you have
  // just plugged the board in and cannot remember where the graphs live.
  if (bootCount <= 1) {
    LOGF("ts: graphs at https://thingspeak.com/channels/%s\n", TS_CHANNEL_ID);
  }

  static char body[384];
  int n = snprintf(body, sizeof(body), "api_key=%s", TS_WRITE_API_KEY);

  n = tsAppendFloat(body, sizeof(body), n, r.soilValid,    "field1", r.soilPct,      1);
  n = tsAppendFloat(body, sizeof(body), n, r.climateValid, "field2", r.temperatureC, 2);
  n = tsAppendFloat(body, sizeof(body), n, r.climateValid, "field3", r.humidityPct,  1);
  n = tsAppendFloat(body, sizeof(body), n, r.climateValid, "field4", r.pressureHpa,  1);
  n = tsAppendFloat(body, sizeof(body), n, r.lightValid,   "field5", r.lux,          1);
  n = tsAppendFloat(body, sizeof(body), n, b.trustworthy,  "field6",
                    (float)b.milliVolts / 1000.0f, 3);
  n = tsAppendInt  (body, sizeof(body), n, true,           "field7", (long)netRssi());
  n = tsAppendInt  (body, sizeof(body), n, r.soilValid,    "field8", (long)r.soilRaw);

  // The two numbers that did not fit in eight fields. The boot counter is the
  // useful one: a channel whose status jumps back to a low number is a board
  // that reset when it should have been asleep, and nothing else would tell me.
  char status[80];
  if (b.trustworthy) {
    snprintf(status, sizeof(status), "boot %u, batt %.0f%%",
             (unsigned)bootCount, b.percent);
  } else {
    snprintf(status, sizeof(status), "boot %u, batt not reported (USB host)",
             (unsigned)bootCount);
  }

  if ((size_t)n < sizeof(body)) {
    int m = snprintf(body + n, sizeof(body) - n, "&status=");
    if (m > 0) {
      n += m;
      if ((size_t)n > sizeof(body)) n = (int)sizeof(body);
      n = tsAppendEncoded(body, sizeof(body), n, status);
    }
  }

  if (n < 0 || (size_t)n >= sizeof(body)) {
    LOGLN("ts: body overflowed - not sent");
    return false;
  }

  char url[96];
  snprintf(url, sizeof(url), "%s://%s/update", TS_USE_TLS ? "https" : "http",
           TS_HOST);

  HTTPClient http;
  bool begun;

#if TS_USE_TLS
  WiFiClientSecure tls;
  tls.setInsecure();
  tls.setTimeout(TS_TIMEOUT_MS / 1000);
  begun = http.begin(tls, url);
#else
  // No handshake, no certificate chain, no 1-2 s of radio at full power. See
  // TS_USE_TLS in config.h for why that is the right trade for this payload.
  WiFiClient plain;
  begun = http.begin(plain, url);
#endif

  if (!begun) {
    LOGLN("ts: begin() failed - bad URL?");
    return false;
  }

  http.setTimeout(TS_TIMEOUT_MS);
  http.setConnectTimeout(TS_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int  code = http.POST((uint8_t *)body, (size_t)n);
  bool ok   = false;

  if (code == 200) {
    // ThingSpeak answers with the new entry number, and with a plain "0" when
    // it has accepted the request and thrown it away - a wrong write key, or an
    // update inside the TS_MIN_INTERVAL_S window. Both are still HTTP 200, so
    // the status code alone will happily tell you everything is fine.
    long entry = http.getString().toInt();
    ok = (entry > 0);
    if (ok) {
      LOGF("ts: entry %ld\n", entry);
    } else {
      LOGF("ts: refused - wrong write key, or inside the %d s rate limit\n",
           TS_MIN_INTERVAL_S);
    }
  } else if (code > 0) {
    LOGF("ts: HTTP %d - %s\n", code, http.getString().c_str());
  } else {
    LOGF("ts: no reply (%d) - DNS, firewall or captive portal?\n", code);
  }

  http.end();
  return ok;
}

#else   // !USE_THINGSPEAK

bool netPublishThingSpeak(const Readings &, const BatteryState &, uint32_t) {
  return false;
}

#endif  // USE_THINGSPEAK

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
