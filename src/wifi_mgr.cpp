#include "wifi_mgr.h"
#include "rsu_config.h"
#include "config.h"
#include <WiFi.h>
#include <Arduino.h>

static unsigned long _lastAttempt = 0;

void wifi_init() {
  if (strlen(cfg_wifi_ssid()) == 0) {
    Serial.println("[WiFi] No SSID configured — skipping.");
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(cfg_wifi_ssid(), cfg_wifi_pass());
  _lastAttempt = millis();
  Serial.print("[WiFi] Connecting to "); Serial.println(cfg_wifi_ssid());
}

bool wifi_ready() {
  if (WiFi.status() == WL_CONNECTED) return true;

  // Periodic reconnect if dropped
  if (millis() - _lastAttempt >= WIFI_RETRY_MS) {
    _lastAttempt = millis();
    Serial.println("[WiFi] Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(cfg_wifi_ssid(), cfg_wifi_pass());
  }
  return false;
}
