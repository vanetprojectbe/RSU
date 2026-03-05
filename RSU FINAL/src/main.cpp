
#include <Arduino.h>
#include "rsu_config.h"
#include "wifi_mgr.h"
#include "sim800.h"
#include "lora_vanet.h"
#include "sd_queue.h"
#include "ota_key.h"
#include "heartbeat.h"

void setup(){
  Serial.begin(115200);
  delay(500);
  cfg_init();
  wifi_init();
  sdq_init();
  sim800_init();
  lora_init();
  ota_key_init();
  heartbeat_init();
}

void loop(){
  lora_poll();
  sdq_retry();
  ota_key_poll();
  heartbeat_poll();
  delay(20);
}
