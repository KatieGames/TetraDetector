#include "display.h"
#include "data.h"
#include <Arduino.h>

static uint32_t lastDisplay = 0;
static bool startupDone = false;

// pulse tracking states
static uint32_t accumulatedPulses = 0;
static bool lastPinState = HIGH;
static uint32_t lastStateChangeTime = 0;

void setup() 
{
    Serial.begin(115200);

    displayInit();
    delay(1000);
    displayStart();
    delay(500);
    displaySafe();

    dataInit();  // initialize data logic for buffering and peak holds

    // SPEED INPUT PIN A5
    pinMode(A5, INPUT_PULLUP); // use internal pullup for speed reed switch to hold line stable
    
    // read initial speed values
    lastPinState = digitalRead(A5);
    lastStateChangeTime = millis();
}

void loop() 
{
    // fast ADC sampling
    uint16_t adc = analogRead(A2);
    tetraAddSample(adc);

    // Read the switch state directly to track changes and prevent interrupt chatter
    bool currentPinState = digitalRead(A5);
    if (currentPinState != lastPinState) 
    {
        // software debounce window to ensure switch contacts settled cleanly
        if (millis() - lastStateChangeTime > 5) 
        {
            // only increment on the transition change (going from LOW to HIGH)
            if (lastPinState == LOW && currentPinState == HIGH) 
            {
                accumulatedPulses++;
            }
            lastPinState = currentPinState;
            lastStateChangeTime = millis();
        }
    }

    // update display every 50 ms
    if (millis() - lastDisplay >= 50) 
    {
        // pass the atomic loop total over to calculation arrays and clear local cache
        uint32_t loopPulses = accumulatedPulses;
        accumulatedPulses = 0;

        speedAddSample(loopPulses);

        int16_t dbm = (int16_t)round(dataGetDbm());
        int16_t speed = (int16_t)round(dataGetSpeed());
        
        updateTextAndBar(dbm, speed);
        lastDisplay = millis();
    }

    // reinit the display once after 15s incase of weird power issues
    // if (!startupDone && millis() >= 15000 && millis() < 15100) 
    // {
    //     displayInit();
    //     displaySafe();
    //     startupDone = true;
    // }
}