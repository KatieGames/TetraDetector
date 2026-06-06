#include "display.h"
#include "data.h"
#include <Arduino.h>

static uint32_t lastDisplay = 0;
static bool startupDone = false;

// pulse counter from speed sensor
volatile uint32_t speedPulses = 0;

// using interrupts
void speedISR()
{
    // mechanical reed switch debounce (ignore triggers within 2000 microseconds of the last valid one)
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = micros();

    if (interrupt_time - last_interrupt_time > 2000) 
    {
        speedPulses++;
        last_interrupt_time = interrupt_time;
    }
}

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

    // SPEED INPUT PIN A3
    pinMode(A3, INPUT_PULLUP); // Use internal pullup for reed switches to hold line stable

    // interrupt on rising edge of square wave for speed
    attachInterrupt(digitalPinToInterrupt(A3), speedISR, RISING);
}

void loop() 
{
    // fast ADC sampling
    uint16_t adc = analogRead(A2);
    tetraAddSample(adc);

    // update display every 10 ms
    if (millis() - lastDisplay >= 10) 
    {
        // copy and reset atomically inside the timed block for steady sample intervals
        uint32_t pulses;
        noInterrupts();
        pulses = speedPulses;
        speedPulses = 0;
        interrupts();

        // convert pulses into whatever your system expects
        speedAddSample(pulses);

        int16_t dbm = (int16_t)round(dataGetDbm());
        int16_t speed = (int16_t)round(dataGetSpeed());
        updateTextAndBar(dbm, speed);
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