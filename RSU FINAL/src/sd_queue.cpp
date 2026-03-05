
#include <Arduino.h>
#include "sd_queue.h"
#include <SD.h>
#include "config.h"
#include "uplink.h"

void sdq_init(){ SD.begin(SD_CS); }

void sdq_enqueue(const char* json){
File f = SD.open("/queue.ndjson", FILE_WRITE);
  if(f){ f.println(json); f.close(); }
}

void sdq_retry(){
  File f=SD.open("/queue.ndjson");
  if(!f) return;
  String line=f.readStringUntil('\n');
  f.close();
  if(line.length()==0) return;
  if(uplink_post("/incidents/report", line.c_str())) SD.remove("/queue.ndjson");
}
