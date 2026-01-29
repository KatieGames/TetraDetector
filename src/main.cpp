#include "display.h"
#include "data.h"
#include <Arduino.h>

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

void loop() {
    static uint32_t lastDisplay = 0;
    static bool startupDone = false;

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

    // reinit the display once after 10s incase of weird power issues
    if (!startupDone && millis() >= 15000) 
    {
        displayInit();
        startupDone = true;
    }
}