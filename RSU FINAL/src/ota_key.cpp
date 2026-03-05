
#include "ota_key.h"
#include "uplink.h"
#include "rsu_config.h"

unsigned long last=0;

void ota_key_init(){ last=millis(); }

void ota_key_poll(){
  if(millis()-last<300000) return;
  last=millis();
  String r;
  if(uplink_get("/rsu/key", r) && r.indexOf("newKey")>0){
    int i=r.indexOf("newKey");
    String k=r.substring(i+9,i+9+64);
    cfg_set_api_key(k.c_str());
  }
}
