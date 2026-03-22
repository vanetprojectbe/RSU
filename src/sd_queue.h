#pragma once
#include <Arduino.h>

void sdq_init();
void sdq_enqueue(const char* json);
void sdq_retry();
uint32_t sdq_pending_count();
