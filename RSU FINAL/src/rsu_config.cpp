
#include "rsu_config.h"
#include <Preferences.h>

static Preferences prefs;

// ===== USER CONFIG =====
static String wifi_ssid = "SAM";
static String wifi_pass = "9892527937";
// =======================

static String apiKey;
static String cmsUrl;

void cfg_init() {
  prefs.begin("rsu", false);
  apiKey = prefs.getString("apiKey", "CHANGE_ME");
  cmsUrl = prefs.getString("cmsUrl", "https://your-cms.vercel.app/api");

  if (!prefs.isKey("wifi_ssid")) {
    prefs.putString("wifi_ssid", wifi_ssid);
    prefs.putString("wifi_pass", wifi_pass);
  }

  wifi_ssid = prefs.getString("wifi_ssid");
  wifi_pass = prefs.getString("wifi_pass");
}

const char* cfg_wifi_ssid(){ return wifi_ssid.c_str(); }
const char* cfg_wifi_pass(){ return wifi_pass.c_str(); }
const char* cfg_api_key(){ return apiKey.c_str(); }
const char* cfg_cms_url(){ return cmsUrl.c_str(); }

void cfg_set_api_key(const char* k){
  apiKey = k;
  prefs.putString("apiKey", apiKey);
}
