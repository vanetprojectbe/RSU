#pragma once
#include <Arduino.h>

bool uplink_post(const char*, const char*);
bool uplink_get(const char*, String&);