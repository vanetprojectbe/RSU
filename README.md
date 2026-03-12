VANET Roadside Unit (RSU)

**ESP32-based Roadside Unit (RSU) for the VANET Accident Detection System.**

The RSU acts as the edge relay node, receiving accident alerts from vehicles over **LoRa VANET** and forwarding them to the **Cloud Management System (CMS)** using **GSM (SIM800L)** with a **Wi-Fi fallback** mechanism.

The system implements store-and-forward logging, OTA API key rotation, and heartbeat monitoring to ensure high reliability in critical emergency scenarios.

---

## System Overview

The RSU is responsible for bridging the Vehicular Ad-hoc Network (VANET) and the Cloud backend.

```text
Vehicle (OBU)
     │
     │ LoRa (VANET)
     ▼
RSU (ESP32)
 ├── LoRa SX1278 Receiver
 ├── SIM800L GSM Uplink (Primary)
 ├── Wi-Fi Uplink (Fallback)
 ├── SD Card Store-and-Forward
 └── CMS API Communication
     │
     ▼
Cloud Monitoring System

```

---

## Hardware Components

| Component | Purpose |
| --- | --- |
| **ESP32 Dev Board** | Main RSU Controller |
| **LoRa SX1278** | VANET radio receiver (433/868/915 MHz) |
| **SIM800L** | GSM/GPRS cloud uplink (Primary) |
| **Micro SD Module** | Store-and-forward queue for offline logging |
| **Antennas** | High-gain LoRa and GSM antennas |

---

## Key Features

* **LoRa VANET Reception:** Real-time monitoring of emergency packets from OBUs.
* **Dual-Path Uplink:** Automatic GSM → Wi-Fi fallback if cellular signal is lost.
* **Store-and-Forward:** Local SD card queuing prevents data loss during network outages.
* **Authenticated API:** Secure communication with CMS via API keys.
* **OTA Key Rotation:** Support for updating security credentials remotely.
* **Heartbeat Monitoring:** Periodic RSU health pings to the central server.

---

## Pin Configuration

### LoRa SX1278 (SPI)

| LoRa Pin | ESP32 Pin |
| --- | --- |
| **VCC** | 3.3V |
| **GND** | GND |
| **SCK** | GPIO 18 |
| **MISO** | GPIO 19 |
| **MOSI** | GPIO 23 |
| **NSS** | GPIO 5 |
| **RST** | GPIO 14 |
| **DIO0** | GPIO 26 |

### SIM800L GSM Module (UART)

| SIM800L Pin | ESP32 Pin |
| --- | --- |
| **VCC** | External 4.0V - 4.2V |
| **GND** | GND |
| **TX** | GPIO 16 (RX2) |
| **RX** | GPIO 17 (TX2) |

> [!IMPORTANT]
> **Power Supply Warning:** The SIM800L requires a peak current of **2A**. Do **NOT** power it directly from the ESP32 3.3V/5V pins. Use an external buck converter (e.g., LM2596) set to 4V.

### SD Card Module (Shared SPI)

| SD Pin | ESP32 Pin |
| --- | --- |
| **VCC** | Vin / 5V |
| **CS** | GPIO 15 |
| **SCK** | GPIO 18 |
| **MOSI** | GPIO 23 |
| **MISO** | GPIO 19 |

---

## Project Structure

```bash
RSU_FINAL_HARDENED_DUAL/
├── platformio.ini         # Environment and dependency config
├── include/
│   └── config.h           # Global definitions
└── src/
    ├── main.cpp           # Main execution loop
    ├── rsu_config.cpp     # Network/API settings
    ├── sim800.cpp         # GSM driver logic
    ├── lora_vanet.cpp     # LoRa packet handling
    ├── sd_queue.cpp       # Store-and-forward logic
    ├── uplink.cpp         # Logic for GSM/Wi-Fi switching
    ├── heartbeat.cpp      # Health check system
    └── ota_key.cpp        # API Key management

```

---

## Installation & Deployment

### 1. Configuration

Update your credentials in `src/rsu_config.cpp`:

```cpp
static String wifi_ssid = "YOUR_WIFI_NAME";
static String wifi_pass = "YOUR_WIFI_PASSWORD";
static String apiKey    = "RSU_SECRET_KEY";
static String cmsUrl    = "https://your-cms.vercel.app/api";

```

### 2. Build and Upload

This project uses **PlatformIO**. Run the following commands:

```bash
# Build the project
pio run

# Upload to ESP32
pio run -t upload

# Open Serial Monitor (115200 baud)
pio device monitor

```

### 3. Verification

Upon boot, the Serial Monitor should display:

```text
[BOOT] RSU Initializing...
[INIT] SD Card Mounted
[INIT] LoRa Radio Ready
[NET]  SIM800L Initialized
[NET]  WiFi Connected (Fallback Ready)
[SYS]  Heartbeat Started

```

---

## Security & Reliability

* **API Authentication:** Every request is sent with an `x-api-key` header verified by the CMS.
* **Data Integrity:** Accident logs are stored in `.ndjson` format on the SD card to ensure recovery after a crash.
* **Failover:** If the GSM GPRS connection fails, the system immediately attempts to route data through the configured Wi-Fi SSID.

---

## License

**Academic Research Project**
*Part of the Vehicular Ad-hoc Network (VANET) Accident Detection & Central Monitoring System.*

---

**Would you like me to generate the corresponding README for the OBU (STM32-based vehicle unit) to complete your documentation?**
