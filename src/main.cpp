#include "display.h"
#include "data.h"
#include <Arduino.h>

static uint32_t lastDisplay = 0;
static bool startupDone = false;

void setup() 
{
    Serial.begin(115200);

    displayInit();
    displayStealth();
    delay(1000);
    displayStart();
    delay(500);
    displaySafe();

    dataInit();  // initialize data logic for buffering and peak holds
}

void loop() 
{
    // sample adc at full speed
    uint16_t adc = analogRead(A2);
    dataAddSample(adc);

    // update display every 10 ms
    if (millis() - lastDisplay >= 10) 
    {
        int16_t dbm = (int16_t)round(dataGetDbm());
        updateDBAndBar(dbm);
        lastDisplay = millis();
    }

    // Potentially fixed issue with variable declaration location?
    // reinit the display once after 15s incase of weird power issues
    if (!startupDone && millis() >= 15000) 
    {
        displayInit();
        displaySafe();
        startupDone = true;
    }
}