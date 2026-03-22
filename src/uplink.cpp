#include "uplink.h"
#include "sim800.h"
#include "wifi_mgr.h"
#include "rsu_config.h"
#include <HTTPClient.h>
#include <Arduino.h>

// ── POST ──────────────────────────────────────────────────────────────────────
bool uplink_post(const char* path, const char* json) {

  // Primary: GSM
  if (sim800_is_registered()) {
    if (sim800_http_post(path, json)) {
      Serial.println("[Uplink] Sent via GSM.");
      return true;
    }
    Serial.println("[Uplink] GSM failed — trying WiFi fallback.");
  }

  // Fallback: WiFi
  if (wifi_ready()) {
    HTTPClient http;
    String url = String(cfg_cms_url()) + path;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key",    cfg_api_key());
    http.setTimeout(10000);

    int code = http.POST((uint8_t*)json, strlen(json));
    http.end();

    if (code >= 200 && code < 300) {
      Serial.println("[Uplink] Sent via WiFi.");
      return true;
    }
    Serial.print("[Uplink] WiFi POST failed — HTTP "); Serial.println(code);
  }

  Serial.println("[Uplink] Both GSM and WiFi unavailable.");
  return false;
}

// ── GET ───────────────────────────────────────────────────────────────────────
bool uplink_get(const char* path, String& out) {

  // Primary: GSM
  if (sim800_is_registered()) {
    if (sim800_http_get(path, out)) return true;
  }

  // Fallback: WiFi
  if (wifi_ready()) {
    HTTPClient http;
    String url = String(cfg_cms_url()) + path;
    http.begin(url);
    http.addHeader("x-api-key", cfg_api_key());
    http.setTimeout(10000);

    int code = http.GET();
    if (code == 200) {
      out = http.getString();
      http.end();
      return true;
    }
    http.end();
  }

  return false;
}
