#include "heartbeat.h"
#include "config.h"
#include "uplink.h"
#include "rsu_config.h"
#include "sd_queue.h"
#include <ArduinoJson.h>
#include <Arduino.h>

static unsigned long _last = 0;

void heartbeat_init() { _last = millis(); }

void heartbeat_poll() {
  if (millis() - _last < HEARTBEAT_MS) return;
  _last = millis();

  StaticJsonDocument<200> doc;
  doc["rsuId"]     = cfg_rsu_id();
  doc["status"]    = "ok";
  doc["uptime_s"]  = millis() / 1000;
  doc["pending"]   = sdq_pending_count();   // unsent records in SD queue

  char buf[200];
  serializeJson(doc, buf, sizeof(buf));

  if (uplink_post(CMS_PATH_HEARTBEAT, buf)) {
    Serial.print("[HB] Heartbeat sent — uptime ");
    Serial.print(millis() / 1000); Serial.println("s");
  } else {
    Serial.println("[HB] Heartbeat failed — no uplink.");
  }
}
