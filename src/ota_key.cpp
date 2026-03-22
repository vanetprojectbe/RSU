#include "ota_key.h"
#include "config.h"
#include "uplink.h"
#include "rsu_config.h"
#include <ArduinoJson.h>
#include <Arduino.h>

static unsigned long _last = 0;

void ota_key_init() { _last = millis(); }

void ota_key_poll() {
  if (millis() - _last < OTA_KEY_MS) return;
  _last = millis();

  String resp;
  if (!uplink_get(CMS_PATH_KEY, resp)) return;

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, resp);
  if (err) {
    Serial.print("[OTA] JSON error: "); Serial.println(err.c_str());
    return;
  }

  const char* newKey = doc["newKey"];
  if (newKey && strlen(newKey) > 8) {
    cfg_set_api_key(newKey);
    Serial.println("[OTA] API key rotated.");
  }
}
