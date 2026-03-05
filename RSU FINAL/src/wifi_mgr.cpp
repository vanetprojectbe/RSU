
#include "wifi_mgr.h"
#include <WiFi.h>
#include "rsu_config.h"

void wifi_init() {
  if (strlen(cfg_wifi_ssid()) == 0) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg_wifi_ssid(), cfg_wifi_pass());
}

bool wifi_ready() {
  return WiFi.status() == WL_CONNECTED;
}
