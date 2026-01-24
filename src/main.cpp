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

    // sample adc at full speed
    uint16_t adc = analogRead(A2); // tetra detector pin
    dataAddSample(adc);

    // do display every 10ms
    if (millis() - lastDisplay >= 10) {
        int16_t dbm = (int16_t)round(dataGetDbm());
        updateDBAndBar(dbm);
        lastDisplay = millis();
    }
}
