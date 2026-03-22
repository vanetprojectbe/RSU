#include "lora_vanet.h"
#include "config.h"
#include "sd_queue.h"
#include "uplink.h"
#include "rsu_config.h"
#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include <ArduinoJson.h>

// ── Init ──────────────────────────────────────────────────────────────────────
void lora_init() {
  // LoRa uses VSPI (default SPI — GPIO18/19/23)
  // SPI.begin() not needed — LoRa library handles VSPI internally
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("[LoRa] Init FAILED — check antenna and wiring!");
    Serial.println("  NSS=GPIO5  RST=GPIO27  DIO0=GPIO26");
    return;
  }

  // ── Radio parameters — must match OBU exactly ────────────────────────────
  LoRa.setSpreadingFactor(LORA_SF);      // SF7
  LoRa.setSignalBandwidth(LORA_BW);      // 125 kHz
  LoRa.setCodingRate4(LORA_CR);          // 4/5
  LoRa.setSyncWord(LORA_SYNC_WORD);      // 0x34 — private VANET channel

  Serial.println("[LoRa] SX1278 ready on VSPI (SCK=18 MISO=19 MOSI=23).");
  Serial.print("  Freq   : "); Serial.print(LORA_FREQ / 1E6, 0); Serial.println(" MHz");
  Serial.print("  SF     : "); Serial.println(LORA_SF);
  Serial.print("  BW     : 125 kHz");    Serial.println();
  Serial.print("  CR     : 4/5");        Serial.println();
  Serial.print("  SyncW  : 0x");         Serial.println(LORA_SYNC_WORD, HEX);
  Serial.print("  Pins   : NSS="); Serial.print(LORA_SS);
  Serial.print(" RST="); Serial.print(LORA_RST);
  Serial.print(" DIO0="); Serial.println(LORA_DIO0);
}

// ── Poll ──────────────────────────────────────────────────────────────────────
// Called every loop iteration — non-blocking.
// OBU sends raw JSON: {"lat":...,"lon":...,"acc":...,...}
void lora_poll() {
  int pktSize = LoRa.parsePacket();
  if (!pktSize) return;

  // Read packet
  String msg = "";
  msg.reserve(pktSize + 8);
  while (LoRa.available()) msg += (char)LoRa.read();
  msg.trim();

  if (msg.length() == 0) return;

  // Validate — must be JSON object from OBU
  if (!msg.startsWith("{")) {
    Serial.print("[LoRa] Non-JSON packet ignored: ");
    Serial.println(msg.substring(0, 40));
    return;
  }

  int   rssi = LoRa.packetRssi();
  float snr  = LoRa.packetSnr();

  Serial.println("[LoRa] ══ EAM RECEIVED ══════════════════════════");
  Serial.print("  RSSI  : "); Serial.print(rssi);    Serial.println(" dBm");
  Serial.print("  SNR   : "); Serial.print(snr, 1);  Serial.println(" dB");
  Serial.print("  Bytes : "); Serial.println(pktSize);
  Serial.println(msg);
  Serial.println("[LoRa] ════════════════════════════════════════════");

  // ── Inject rsuId so CMS knows which RSU forwarded this ────────────────────
  // Parse the OBU JSON, add rsuId, re-serialize
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, msg);

  String payload;
  if (!err) {
    doc["rsuId"] = cfg_rsu_id();
    serializeJson(doc, payload);
  } else {
    // JSON parse failed — forward raw payload anyway
    payload = msg;
    Serial.print("[LoRa] JSON parse warning: "); Serial.println(err.c_str());
  }

  // ── Step 1: Persist to SD first ───────────────────────────────────────────
  // Guarantees no data loss even if uplink fails immediately after
  sdq_enqueue(payload.c_str());

  // ── Step 2: Attempt immediate uplink to CMS ───────────────────────────────
  if (uplink_post(CMS_PATH_ACCIDENTS, payload.c_str())) {
    Serial.println("[LoRa] EAM forwarded to CMS.");
  } else {
    Serial.println("[LoRa] Uplink failed — EAM in SD queue for retry.");
  }
}
