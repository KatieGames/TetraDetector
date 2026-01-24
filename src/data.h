#pragma once
#include <Arduino.h>

void dataInit();
void dataAddSample(uint16_t adc);
float dataGetDbm();
