#pragma once

// ════════════════════════════════════════════════════════════════════════════════
//  RSU — Master Configuration
//  Board  : ESP32 Dev Board
//  Partner: OBU — STM32F411 Black Pill (github.com/vanetprojectbe/OBU)
//  Backend: CMS  — https://cms-backend-lmof.onrender.com
//  ML     : https://vanet-ml-service.onrender.com
//
//  SPI buses (separate — zero contention):
//    VSPI (default) ─── LoRa SX1278   SCK=18  MISO=19  MOSI=23
//    HSPI           ─── SD card       SCK=14  MISO=12  MOSI=13
// ════════════════════════════════════════════════════════════════════════════════

// ── LoRa SX1278 — VSPI ───────────────────────────────────────────────────────
// ⚠ ALL values MUST match OBU firmware exactly — any mismatch = no link
#define LORA_FREQ          433E6   // 433 MHz — OBU transmits at 433 MHz
#define LORA_SYNC_WORD     0x34   // Private VANET channel — matches OBU config.h
#define LORA_SF            7      // Spreading Factor 7 — matches OBU
#define LORA_BW            125E3  // Bandwidth 125 kHz — matches OBU
#define LORA_CR            5      // Coding Rate 4/5 — matches OBU
#define LORA_TX_POWER      17     // dBm (OBU reference, RSU is RX only)

// LoRa pin assignments — VSPI bus
#define LORA_SS            5      // NSS / chip select
#define LORA_RST           27     // Reset — GPIO27 (NOT 14, HSPI uses 14 for SCK)
#define LORA_DIO0          26     // Interrupt / packet ready

// ── SD card — HSPI (dedicated, isolated from LoRa) ───────────────────────────
#define HSPI_SCK           14
#define HSPI_MISO          12
#define HSPI_MOSI          13
#define SD_CS              15
#define SD_QUEUE_FILE      "/queue.ndjson"   // NDJSON — one JSON line per record

// ── SIM800L — UART2 ──────────────────────────────────────────────────────────
// ⚠ SIM800L needs external 4.0V @ 2A supply — NEVER power from ESP32 pins
#define SIM_RX             16    // ESP32 RX2 ← SIM800L TX
#define SIM_TX             17    // ESP32 TX2 → SIM800L RX

// ── CMS API endpoints (paths only — base URL in rsu_config.cpp) ──────────────
#define CMS_PATH_ACCIDENTS  "/api/accidents"    // POST — forward OBU EAM
#define CMS_PATH_HEARTBEAT  "/api/rsu/heartbeat"// POST — health ping
#define CMS_PATH_KEY        "/api/rsu/key"      // GET  — OTA key rotation

// ── Timing ───────────────────────────────────────────────────────────────────
#define LOOP_DELAY_MS      20       // ms between loop iterations
#define SD_RETRY_MS        10000UL  // 10 s — SD queue retry interval
#define HEARTBEAT_MS       60000UL  // 60 s — CMS heartbeat interval
#define OTA_KEY_MS         300000UL // 5 min — API key rotation check
#define WIFI_RETRY_MS      30000UL  // 30 s — WiFi reconnect attempt interval
#define SIM_REGISTER_TRIES 10       // attempts to wait for GSM registration
#define SIM_REGISTER_DELAY 2000     // ms between registration checks
