
#include "uplink.h"
#include "sim800.h"
#include "wifi_mgr.h"
#include <HTTPClient.h>
#include "rsu_config.h"

bool uplink_post(const char* path, const char* json) {
  if (sim800_is_registered()) return sim800_http_post(path, json);
  if (wifi_ready()) {
    HTTPClient http;
    String url = String(cfg_cms_url()) + path;
    http.begin(url);
    http.addHeader("Content-Type","application/json");
    http.addHeader("x-api-key",cfg_api_key());
    int code = http.POST((uint8_t*)json, strlen(json));
    http.end();
    return code>=200 && code<300;
  }
  return false;
}

bool uplink_get(const char* path, String& out) {
  if (sim800_is_registered()) return sim800_http_get(path, out);
  if (wifi_ready()) {
    HTTPClient http;
    String url = String(cfg_cms_url()) + path;
    http.begin(url);
    http.addHeader("x-api-key",cfg_api_key());
    int code=http.GET();
    if(code==200) out=http.getString();
    http.end();
    return code==200;
  }
  return false;
}
