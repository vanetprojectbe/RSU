#pragma once

// Load all credentials from NVS flash
void        cfg_init();

// Accessors
const char* cfg_wifi_ssid();
const char* cfg_wifi_pass();
const char* cfg_api_key();
const char* cfg_cms_url();
const char* cfg_apn();
const char* cfg_rsu_id();

// Update API key (called by OTA key rotation)
void cfg_set_api_key(const char* key);
