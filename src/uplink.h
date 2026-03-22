#pragma once
#include <Arduino.h>

// Forward JSON payload to CMS via GSM (primary) or WiFi (fallback)
bool uplink_post(const char* path, const char* json);

// GET request from CMS via GSM or WiFi
bool uplink_get(const char* path, String& out);
