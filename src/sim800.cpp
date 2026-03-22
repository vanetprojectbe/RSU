#include "sim800.h"
#include "config.h"
#include "rsu_config.h"
#include <HardwareSerial.h>
#include <Arduino.h>

static HardwareSerial _sim(2);   // UART2 — GPIO16=RX, GPIO17=TX

// ── AT command helper ─────────────────────────────────────────────────────────
static bool at(const char* cmd, const char* expect = "OK", unsigned long timeout = 3000) {
  _sim.println(cmd);
  String resp = "";
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (_sim.available()) resp += (char)_sim.read();
    if (resp.indexOf(expect) >= 0) return true;
    if (resp.indexOf("ERROR") >= 0) {
      Serial.print("[SIM] ERROR on: "); Serial.println(cmd);
      return false;
    }
  }
  return false;
}

// ── GPRS bearer setup ─────────────────────────────────────────────────────────
static bool gprs_init() {
  if (!at("AT+CGATT=1", "OK", 10000)) {
    Serial.println("[SIM] GPRS attach failed.");
    return false;
  }

  at("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");

  String apnCmd = "AT+SAPBR=3,1,\"APN\",\"";
  apnCmd += cfg_apn();
  apnCmd += "\"";
  at(apnCmd.c_str());

  if (!at("AT+SAPBR=1,1", "OK", 10000)) {
    Serial.println("[SIM] Bearer open failed.");
    return false;
  }

  return at("AT+SAPBR=2,1", "OK");
}

// ── Public: init ──────────────────────────────────────────────────────────────
bool sim800_init() {
  _sim.begin(115200, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(1000);

  // Auto-baud sync — try up to 5 times
  bool alive = false;
  for (int i = 0; i < 5; i++) {
    if (at("AT")) { alive = true; break; }
    delay(500);
  }
  if (!alive) {
    Serial.println("[SIM] No response — check wiring and power supply (4V/2A).");
    return false;
  }

  at("ATE0");        // echo off
  at("AT+CMEE=2");   // verbose errors

  // Wait for network registration
  bool registered = false;
  for (int i = 0; i < SIM_REGISTER_TRIES; i++) {
    if (sim800_is_registered()) { registered = true; break; }
    Serial.print("[SIM] Waiting for network... "); Serial.println(i + 1);
    delay(SIM_REGISTER_DELAY);
  }

  if (!registered) {
    Serial.println("[SIM] Network registration failed — check SIM card and APN.");
    return false;
  }

  if (!gprs_init()) {
    Serial.println("[SIM] GPRS init failed.");
    return false;
  }

  Serial.println("[SIM] GPRS ready.");
  return true;
}

bool sim800_is_registered() {
  _sim.println("AT+CREG?");
  delay(300);
  String resp = _sim.readString();
  return (resp.indexOf(",1") >= 0 || resp.indexOf(",5") >= 0);
}

// ── HTTP POST via GSM ─────────────────────────────────────────────────────────
bool sim800_http_post(const char* path, const char* json) {
  String url = String(cfg_cms_url()) + path;

  at("AT+HTTPTERM");
  if (!at("AT+HTTPINIT")) return false;

  at("AT+HTTPPARA=\"CID\",1");

  String cmd = "AT+HTTPPARA=\"URL\",\"" + url + "\"";
  at(cmd.c_str());

  cmd = "AT+HTTPPARA=\"USERDATA\",\"x-api-key: ";
  cmd += cfg_api_key();
  cmd += "\"";
  at(cmd.c_str());

  at("AT+HTTPPARA=\"CONTENT\",\"application/json\"");

  _sim.print("AT+HTTPDATA=");
  _sim.print(strlen(json));
  _sim.println(",5000");

  if (!_sim.find("DOWNLOAD")) {
    at("AT+HTTPTERM");
    Serial.println("[SIM] HTTPDATA download prompt not received.");
    return false;
  }

  _sim.print(json);
  delay(500);

  if (!at("AT+HTTPACTION=1", "200", 15000)) {
    at("AT+HTTPTERM");
    Serial.println("[SIM] POST action failed or non-200 response.");
    return false;
  }

  at("AT+HTTPTERM");
  return true;
}

// ── HTTP GET via GSM ──────────────────────────────────────────────────────────
bool sim800_http_get(const char* path, String& out) {
  String url = String(cfg_cms_url()) + path;

  at("AT+HTTPTERM");
  if (!at("AT+HTTPINIT")) return false;

  String cmd = "AT+HTTPPARA=\"URL\",\"" + url + "\"";
  at(cmd.c_str());

  cmd = "AT+HTTPPARA=\"USERDATA\",\"x-api-key: ";
  cmd += cfg_api_key();
  cmd += "\"";
  at(cmd.c_str());

  if (!at("AT+HTTPACTION=0", "200", 15000)) {
    at("AT+HTTPTERM");
    return false;
  }

  _sim.println("AT+HTTPREAD");
  delay(500);
  out = _sim.readString();
  at("AT+HTTPTERM");
  return true;
}
