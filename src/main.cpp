// ════════════════════════════════════════════════════════════════════════════════
//  RSU — VANET Roadside Unit
//  Board  : ESP32 Dev Board
//  Partner: OBU (STM32F411 Black Pill) + CMS Backend + ML Service
//
//  SPI Buses (separate — zero contention):
//    VSPI  →  LoRa SX1278    SCK=18  MISO=19  MOSI=23  NSS=5   RST=27  DIO0=26
//    HSPI  →  SD card        SCK=14  MISO=12  MOSI=13  CS=15
//
//  Other:
//    UART2 →  SIM800L GSM    RX=16   TX=17   (4.0V external supply required)
//    WiFi  →  ESP32 built-in (fallback uplink)
// ════════════════════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include "config.h"
#include "rsu_config.h"
#include "sd_queue.h"
#include "lora_vanet.h"
#include "wifi_mgr.h"
#include "sim800.h"
#include "uplink.h"
#include "heartbeat.h"
#include "ota_key.h"

static unsigned long _lastSdRetry = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("================================================");
  Serial.println("  RSU — VANET Roadside Unit  v2.0");
  Serial.println("  ESP32 Dev Board");
  Serial.println("  VSPI=LoRa  |  HSPI=SD  |  UART2=GSM");
  Serial.println("================================================");

  // 1 ── Credentials from NVS flash
  cfg_init();

  // 2 ── SD card on HSPI (GPIO 14/12/13/15)
  //      Init before LoRa so queue is ready to accept first packet
  sdq_init();

  // 3 ── LoRa on VSPI (GPIO 18/19/23) — separate from SD, no contention
  lora_init();

  // 4 ── WiFi (fallback uplink — async, connects in background)
  wifi_init();

  // 5 ── GSM primary uplink
  if (!sim800_init()) {
    Serial.println("[WARN] SIM800L unavailable — WiFi-only uplink mode active.");
  }

  // 6 ── Background services
  ota_key_init();
  heartbeat_init();

  Serial.println("================================================");
  Serial.println("[RSU] Ready — monitoring for OBU EAM packets.");
  Serial.println("  Commands: none (serial = debug log only)");
  Serial.println("================================================");
}

void loop() {
  unsigned long now = millis();

  // ── LoRa — check for incoming EAM every 20ms ─────────────────────────────
  lora_poll();

  // ── SD retry — every SD_RETRY_MS (10s) ───────────────────────────────────
  if (now - _lastSdRetry >= SD_RETRY_MS) {
    _lastSdRetry = now;
    sdq_retry();
  }

  // ── Heartbeat — every HEARTBEAT_MS (60s) ─────────────────────────────────
  heartbeat_poll();

  // ── OTA key rotation — every OTA_KEY_MS (5 min) ──────────────────────────
  ota_key_poll();

  delay(LOOP_DELAY_MS);
}
