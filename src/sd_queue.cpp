#include "sd_queue.h"
#include "config.h"
#include "uplink.h"
#include <SPI.h>
#include <SD.h>
#include <Arduino.h>
#include <vector>

// ── Dedicated HSPI bus — completely isolated from LoRa VSPI ──────────────────
static SPIClass _hspi(HSPI);
static bool     _ready = false;

// ── Init ──────────────────────────────────────────────────────────────────────
void sdq_init() {
  _hspi.begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, SD_CS);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  if (!SD.begin(SD_CS, _hspi)) {
    Serial.println("[SD] Mount failed — check wiring (GPIO14/12/13/15) and FAT32 format.");
    _ready = false;
    return;
  }

  _ready = true;
  uint64_t mb = SD.cardSize() / (1024 * 1024);
  Serial.println("[SD] Card mounted on HSPI (SCK=14 MISO=12 MOSI=13 CS=15).");
  Serial.print("  Card size : "); Serial.print(mb); Serial.println(" MB");

  // Show any pending records from previous session
  uint32_t pending = sdq_pending_count();
  if (pending > 0) {
    Serial.print("  Pending   : "); Serial.print(pending); Serial.println(" unsent record(s) from last session.");
  }
}

// ── Enqueue ───────────────────────────────────────────────────────────────────
// Appends one JSON line to the NDJSON queue file.
// Called immediately when a LoRa EAM is received — before uplink attempt.
void sdq_enqueue(const char* json) {
  if (!_ready) {
    Serial.println("[SD] Not ready — EAM not persisted.");
    return;
  }
  File f = SD.open(SD_QUEUE_FILE, FILE_APPEND);
  if (!f) {
    Serial.println("[SD] Cannot open queue file.");
    return;
  }
  f.println(json);
  f.close();
  Serial.println("[SD] EAM persisted to queue.");
}

// ── Retry ─────────────────────────────────────────────────────────────────────
// Reads every line in the queue, attempts uplink for each.
// Lines that succeed are removed; failed lines are kept.
// Called every SD_RETRY_MS from main loop.
void sdq_retry() {
  if (!_ready) return;

  File f = SD.open(SD_QUEUE_FILE, FILE_READ);
  if (!f || f.size() == 0) {
    if (f) f.close();
    return;
  }

  std::vector<String> failed;
  int sent = 0;

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() < 5) continue;

    if (uplink_post(CMS_PATH_ACCIDENTS, line.c_str())) {
      sent++;
    } else {
      failed.push_back(line);
    }
  }
  f.close();

  // Rewrite queue keeping only unsent records
  SD.remove(SD_QUEUE_FILE);
  if (!failed.empty()) {
    File out = SD.open(SD_QUEUE_FILE, FILE_WRITE);
    if (out) {
      for (auto& l : failed) out.println(l);
      out.close();
    }
  }

  if (sent > 0 || !failed.empty()) {
    Serial.print("[SD] Retry — sent: "); Serial.print(sent);
    Serial.print("  still pending: "); Serial.println(failed.size());
  }
}

// ── Count pending ─────────────────────────────────────────────────────────────
uint32_t sdq_pending_count() {
  if (!_ready) return 0;
  File f = SD.open(SD_QUEUE_FILE, FILE_READ);
  if (!f) return 0;
  uint32_t count = 0;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.trim().length() > 5) count++;
  }
  f.close();
  return count;
}
