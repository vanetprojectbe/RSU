
#include "sim800.h"
#include <HardwareSerial.h>
#include "config.h"
#include "rsu_config.h"

static HardwareSerial sim(2);

static bool at(const char* cmd, unsigned long t=3000) {
  sim.println(cmd);
  unsigned long s = millis();
  while (millis()-s < t) {
    if (sim.find("OK")) return true;
    if (sim.find("ERROR")) return false;
  }
  return false;
}

bool sim800_init() {
  sim.begin(115200, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(1000);
  return at("AT") && at("ATE0");
}

bool sim800_is_registered() {
  sim.println("AT+CREG?");
  return sim.find(",1") || sim.find(",5");
}

bool sim800_http_post(const char* path, const char* json) {

  String url = String(cfg_cms_url()) + path;

  at("AT+HTTPTERM");
  if (!at("AT+HTTPINIT")) return false;

  at("AT+HTTPPARA=\"CID\",1");

  String cmd;

  cmd = "AT+HTTPPARA=\"URL\",\"" + url + "\"";
  at(cmd.c_str());

  cmd = "AT+HTTPPARA=\"USERDATA\",\"x-api-key: ";
  cmd += cfg_api_key();
  cmd += "\"";
  at(cmd.c_str());

  at("AT+HTTPPARA=\"CONTENT\",\"application/json\"");

  sim.print("AT+HTTPDATA=");
  sim.print(strlen(json));
  sim.println(",5000");

  if (!sim.find("DOWNLOAD")) return false;

  sim.print(json);
  delay(300);
  sim.println();

  at("AT+HTTPACTION=1", 15000);
  at("AT+HTTPTERM");

  return true;
}

bool sim800_http_get(const char* path, String& out) {

  String url = String(cfg_cms_url()) + path;

  at("AT+HTTPINIT");

  String cmd;

  cmd = "AT+HTTPPARA=\"URL\",\"" + url + "\"";
  at(cmd.c_str());

  cmd = "AT+HTTPPARA=\"USERDATA\",\"x-api-key: ";
  cmd += cfg_api_key();
  cmd += "\"";
  at(cmd.c_str());

  at("AT+HTTPACTION=0", 15000);

  sim.println("AT+HTTPREAD");
  out = sim.readString();

  at("AT+HTTPTERM");

  return true;
}
