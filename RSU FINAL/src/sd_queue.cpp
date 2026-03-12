#include <Arduino.h>
#include "sd_queue.h"
#include <SD.h>
#include "config.h"
#include "uplink.h"
#include <SPI.h>

void sdq_init(){
  SPI.begin(18, 19, 23, 15);

  if(!SD.begin(SD_CS)){
    Serial.println("❌ SD mount failed");
  } else {
    Serial.println("✅ SD card mounted");
  }
}

void sdq_enqueue(const char* json){
  File f = SD.open("/queue.ndjson", FILE_WRITE);
  if(f){
    f.println(json);
    f.close();
  }
}

void sdq_retry(){
  File f = SD.open("/queue.ndjson");
  if(!f) return;

  String line = f.readStringUntil('\n');
  f.close();

  if(line.length() == 0) return;

  if(uplink_post("/accidents", line.c_str())){
    SD.remove("/queue.ndjson");
  }
}
