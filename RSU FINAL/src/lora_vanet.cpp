
#include <Arduino.h>
#include "lora_vanet.h"
#include <LoRa.h>
#include "config.h"
#include "sd_queue.h"

void lora_init() {
  LoRa.setPins(LORA_SS,LORA_RST,LORA_DIO0);
  LoRa.begin(LORA_FREQ);
}

void lora_poll() {
  int p=LoRa.parsePacket();
  if(!p) return;
  String msg;
  while(LoRa.available()) msg+=(char)LoRa.read();
  if(!msg.startsWith("EAM|")) return;
  sdq_enqueue(msg.c_str());
  LoRa.beginPacket();
  LoRa.print("ACK");
  LoRa.endPacket();
}
