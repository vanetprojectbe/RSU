#include "rsu_config.h"
#include <Preferences.h>
#include <Arduino.h>

static Preferences prefs;

// ════════════════════════════════════════════════════════════════════════════
//  ★ SET THESE BEFORE FIRST FLASH ★
//  After first boot they are saved to NVS flash and persist across reboots.
//  To change credentials after deployment: update here and reflash,
//  OR implement a serial command handler to call prefs.putString() directly.
// ════════════════════════════════════════════════════════════════════════════
static String _wifi_ssid = "YOUR_WIFI_SSID";
static String _wifi_pass = "YOUR_WIFI_PASSWORD";
static String _apn       = "internet";   // Jio: "jionet"  Airtel: "airtelgprs.com"  Vi: "www"  BSNL: "bsnlnet"
static String _rsuId     = "RSU-001";    // unique ID for this unit — change per deployment
// ════════════════════════════════════════════════════════════════════════════

static String _apiKey;
static String _cmsUrl;

void cfg_init() {
  prefs.begin("rsu", false);

  // First boot — write defaults to NVS
  if (!prefs.isKey("wifi_ssid")) {
    prefs.putString("wifi_ssid", _wifi_ssid);
    prefs.putString("wifi_pass", _wifi_pass);
    prefs.putString("apn",       _apn);
    prefs.putString("rsuId",     _rsuId);
    Serial.println("[CFG] First boot — defaults written to NVS flash.");
  }

  // Always read from NVS (picks up any OTA-updated values)
  _wifi_ssid = prefs.getString("wifi_ssid", _wifi_ssid);
  _wifi_pass = prefs.getString("wifi_pass", _wifi_pass);
  _apn       = prefs.getString("apn",       _apn);
  _rsuId     = prefs.getString("rsuId",     _rsuId);
  _apiKey    = prefs.getString("apiKey",    "CHANGE_ME");
  _cmsUrl    = prefs.getString("cmsUrl",    "https://cms-backend-lmof.onrender.com");

  Serial.println("[CFG] Configuration loaded:");
  Serial.print("  RSU ID  : "); Serial.println(_rsuId);
  Serial.print("  CMS URL : "); Serial.println(_cmsUrl);
  Serial.print("  API Key : "); Serial.println(_apiKey.substring(0, 8) + "...");
  Serial.print("  WiFi    : "); Serial.println(_wifi_ssid);
  Serial.print("  APN     : "); Serial.println(_apn);
}

const char* cfg_wifi_ssid() { return _wifi_ssid.c_str(); }
const char* cfg_wifi_pass() { return _wifi_pass.c_str(); }
const char* cfg_api_key()   { return _apiKey.c_str(); }
const char* cfg_cms_url()   { return _cmsUrl.c_str(); }
const char* cfg_apn()       { return _apn.c_str(); }
const char* cfg_rsu_id()    { return _rsuId.c_str(); }

void cfg_set_api_key(const char* key) {
  _apiKey = String(key);
  prefs.putString("apiKey", _apiKey);
  Serial.println("[CFG] API key rotated and saved to NVS.");
}
