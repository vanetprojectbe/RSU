
#include <Arduino.h>
#include "heartbeat.h"
#include "uplink.h"
#include "config.h"

unsigned long lastHb=0;

void heartbeat_init(){ lastHb=millis(); }

void heartbeat_poll(){
  if(millis()-lastHb<HEARTBEAT_MS) return;
  lastHb=millis();
  uplink_post("/rsu/heartbeat","{\"status\":\"ok\"}");
}
