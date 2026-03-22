# RSU — VANET Roadside Unit

ESP32-based edge relay node. Receives accident alerts from OBU over LoRa
and forwards them to the CMS backend via GSM (primary) or WiFi (fallback).

```
OBU (STM32F411)
  └── LoRa 433 MHz ──► RSU (ESP32)
                          ├── SIM800L GSM ──► CMS Backend
                          ├── WiFi fallback ──► CMS Backend
                          └── SD card (store-and-forward queue)
```

---

## Hardware

| Component        | Purpose                   | Notes                          |
|------------------|---------------------------|--------------------------------|
| ESP32 Dev Board  | Main controller           |                                |
| LoRa SX1278      | Receives OBU EAM packets  | 433 MHz, VSPI bus              |
| SIM800L          | GSM/GPRS primary uplink   | 2G only, needs 4V/2A external  |
| Micro SD module  | Store-and-forward queue   | FAT32, HSPI bus                |
| LM2596           | Powers SIM800L at 4V      | Never use ESP32 pins for SIM   |
| LoRa antenna     | Signal reception          | Must be connected before power |
| GSM antenna      | Cellular signal           | Must be connected before power |

---

## Wiring

### VSPI — LoRa SX1278

```
VCC  ──► 3.3V
GND  ──► GND
SCK  ──► GPIO 18
MISO ──► GPIO 19
MOSI ──► GPIO 23
NSS  ──► GPIO 5   (chip select)
RST  ──► GPIO 27  (NOT 14 — HSPI uses 14 for SCK)
DIO0 ──► GPIO 26
```

### HSPI — SD card (dedicated bus)

```
VCC  ──► 3.3V
GND  ──► GND
SCK  ──► GPIO 14
MISO ──► GPIO 12
MOSI ──► GPIO 13
CS   ──► GPIO 15
```

### UART2 — SIM800L

```
VCC  ──► 4.0V from LM2596   (NOT ESP32 pins — SIM800L needs 2A peak)
GND  ──► Common GND
TX   ──► GPIO 16  (ESP32 RX2)
RX   ──► GPIO 17  (ESP32 TX2)
```

### Power

```
12V ──► LM2596 ──► 4.0V ──► SIM800L VCC
                ──► 5.0V ──► ESP32 VIN  (or power ESP32 via USB)
Common GND across all modules
```

---

## Pin Map

| GPIO | Signal    | Module         |
|------|-----------|----------------|
| 5    | VSPI CS   | LoRa NSS       |
| 12   | HSPI MISO | SD card        |
| 13   | HSPI MOSI | SD card        |
| 14   | HSPI SCK  | SD card        |
| 15   | HSPI CS   | SD card        |
| 16   | UART2 RX  | SIM800L TX     |
| 17   | UART2 TX  | SIM800L RX     |
| 18   | VSPI SCK  | LoRa           |
| 19   | VSPI MISO | LoRa           |
| 23   | VSPI MOSI | LoRa           |
| 26   | DIO0      | LoRa interrupt |
| 27   | RST       | LoRa reset     |

---

## Setup before flashing

Edit `src/rsu_config.cpp` and set:

```cpp
static String _wifi_ssid = "YOUR_WIFI_SSID";
static String _wifi_pass = "YOUR_WIFI_PASSWORD";
static String _apn       = "internet";    // Jio: "jionet"  Airtel: "airtelgprs.com"
static String _rsuId     = "RSU-001";     // unique per unit
```

After first boot these are saved to NVS flash and persist across reboots.

The RSU_API_KEY must match `RSU_API_KEY` in the CMS `.env` on Render.
Set it via `cfg_set_api_key()` on first boot (add a one-time serial command).

---

## LoRa radio settings (must match OBU)

| Parameter       | Value  |
|-----------------|--------|
| Frequency       | 433 MHz |
| Spreading Factor| SF7    |
| Bandwidth       | 125 kHz |
| Coding Rate     | 4/5    |
| Sync Word       | 0x34   |

---

## Build and flash

```bash
# Build
pio run

# Flash
pio run -t upload

# Monitor
pio device monitor
```

---

## Expected boot output

```
================================================
  RSU — VANET Roadside Unit  v2.0
  ESP32 Dev Board
  VSPI=LoRa  |  HSPI=SD  |  UART2=GSM
================================================
[CFG] Configuration loaded:
  RSU ID  : RSU-001
  CMS URL : https://cms-backend-lmof.onrender.com
  API Key : XXXXXXXX...
  WiFi    : YOUR_SSID
  APN     : internet
[SD] Card mounted on HSPI (SCK=14 MISO=12 MOSI=13 CS=15).
  Card size : 32 MB
[LoRa] SX1278 ready on VSPI (SCK=18 MISO=19 MOSI=23).
  Freq   : 433 MHz
  SF     : 7
  BW     : 125 kHz
  SyncW  : 0x34
[WiFi] Connecting to YOUR_SSID
[SIM] GPRS ready.
================================================
[RSU] Ready — monitoring for OBU EAM packets.
================================================
```

---

## When EAM received from OBU

```
[LoRa] ══ EAM RECEIVED ══════════════════════════
  RSSI  : -72 dBm
  SNR   : 8.5 dB
  Bytes : 187
{"lat":19.076,"lon":72.877,"acc":18.5,"gyro":320.1,...}
[LoRa] ════════════════════════════════════════════
[SD]  EAM persisted to queue.
[Uplink] Sent via GSM.
[LoRa] EAM forwarded to CMS.
```

---

## CMS endpoints called

| Method | Path                  | Auth        | When              |
|--------|-----------------------|-------------|-------------------|
| POST   | /api/accidents        | x-api-key   | EAM received      |
| POST   | /api/rsu/heartbeat    | x-api-key   | Every 60s         |
| GET    | /api/rsu/key          | x-api-key   | Every 5 min       |

---

## Troubleshooting

| Symptom                    | Fix                                              |
|----------------------------|--------------------------------------------------|
| LoRa init failed           | Check antenna, NSS=5, RST=27, DIO0=26           |
| SD mount failed            | Format SD as FAT32, check GPIO14/12/13/15        |
| SIM no response            | Check 4V supply, wiring GPIO16/17                |
| SIM not registered         | Check APN, SIM card 2G capable (not VoLTE-only)  |
| Nothing received from OBU  | Confirm SF7 and sync word 0x34 match on both     |
| CMS returns 401            | RSU_API_KEY mismatch — check Render .env         |
| CMS returns 404            | Wrong CMS URL in rsu_config.cpp                  |
