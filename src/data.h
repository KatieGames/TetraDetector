#pragma once
#include <Arduino.h>

void dataInit();
void tetraAddSample(uint16_t adc);
void speedAddSample(uint32_t speedPulses);
float dataGetDbm();
float dataGetSpeed();
