#include "display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <screens.h>

// screen properties
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR 0x3c

// bar properties
#define BAR_X 54
#define BAR_Y 20
#define BAR_WIDTH 72
#define BAR_HEIGHT 10

// db text properties
#define DB_TEXT_X 78
#define DB_TEXT_Y 6

// configurable for db scaling
int dbMin = -60;
int dbMax = 5;

// thresholds
int warnDB = -30;   // dbm to go to warn
int stopDB = -20;   // dbm to go to stop

String currentVersion = "v1.1";

// screen states
enum ScreenState 
{
    SAFE,
    WARN,
    STOP,
    STEALTH,
    START
};

ScreenState currentScreenState = SAFE;

// oled display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// smooth bar state
static float smoothedDbm = -90.0f;
static constexpr float barFallFactor = 0.02f; // smaller = slower fall

// stop blink state
static bool stopBlinkOn = true;
static uint16_t stopBlinkCounter = 0;               // frames counter for current blink
static uint16_t stopBlinkRemaining = 0;             // remaining ON-OFF cycles
static constexpr uint16_t stopBlinkInterval = 50;   // frames per blink (~0.5s)
static constexpr uint8_t stopMinCycles = 3;         // minimum flashes per STOP trigger


// initialize display
void displayInit() 
{
    // pins b6 and b7 are used for i2c
    Wire.begin();

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
    {
        while (1); // cannot init display
    }

    display.clearDisplay();
    display.display();
}

// update db text
void displayUpdateDB(int16_t dbValue) 
{
    int textY = DB_TEXT_Y;
    if (textY < 0) textY = 0;
    if (textY > SCREEN_HEIGHT - 8) textY = SCREEN_HEIGHT - 8;

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(DB_TEXT_X, textY);
    display.printf("%+3d dB", dbValue);
}

// draw horizontal bar
void drawBar(int16_t dbValue) 
{
    float fillPercent = (float)(dbValue - dbMin) / (dbMax - dbMin);
    fillPercent = constrain(fillPercent, 0.0f, 1.0f);

    display.fillRect(BAR_X + 2, BAR_Y + 2, BAR_WIDTH - 4, BAR_HEIGHT - 4, SSD1306_BLACK);
    display.drawRect(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, SSD1306_WHITE);

    int fillWidth = (BAR_WIDTH - 4) * fillPercent;
    if (fillWidth > 0) {
        display.fillRect(BAR_X + 2, BAR_Y + 2, fillWidth, BAR_HEIGHT - 4, SSD1306_WHITE);
    }
}

// draw static screens
void displaySafe() 
{
    display.clearDisplay();
    display.drawBitmap(0, 0, safe_screen, 128, 32, SSD1306_WHITE);
    display.display();
}

void displayStealth() 
{
    display.clearDisplay();
    display.drawBitmap(0, 0, stealth_screen, 128, 32, SSD1306_WHITE);
    display.display();
}

void displayStart() 
{
    display.clearDisplay();
    display.drawBitmap(0, 0, start_screen, 128, 32, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(1, 1);
    display.print(currentVersion);
    display.display();
}

void updateDBAndBar(int16_t dbValue) 
{
    // smooth bar: rise immediately, fall gradually
    if (dbValue > smoothedDbm) 
    {
        smoothedDbm = dbValue;
    } 
    else 
    {
        smoothedDbm += (dbValue - smoothedDbm) * barFallFactor;
    }

    // determine requested state based on dbValue
    ScreenState requestedState;
    if (dbValue >= stopDB) 
    {
        requestedState = STOP;
    } 
    else if (dbValue >= warnDB) 
    {
        requestedState = WARN;
    } 
    else 
    {
        requestedState = SAFE;
    }

    // blinks stop
    if (requestedState == STOP) 
    {
        // extend remaining flashes if needed
        if(stopBlinkRemaining < 3)
        {
            stopBlinkRemaining += stopMinCycles; // each trigger adds min flashes
        }   
    }

    bool redraw = false;

    if (stopBlinkRemaining > 0) 
    {
        // currently in stop sequence
        stopBlinkCounter++;
        if (stopBlinkCounter >= stopBlinkInterval) 
        {
            stopBlinkCounter = 0;
            stopBlinkOn = !stopBlinkOn;
            redraw = true;

            // decrement remaining flashes after OFF phase
            if (!stopBlinkOn && stopBlinkRemaining > 0) 
            {
                stopBlinkRemaining--;
            }
        }

        // always show stop while in sequence
        if (redraw || currentScreenState != STOP) 
        {
            currentScreenState = STOP;
            display.clearDisplay();
            if (stopBlinkOn) 
            {
                display.drawBitmap(0, 0, stop_screen, 128, 32, SSD1306_WHITE);
            }
            display.display();
        }
    } 
    else 
    {
        // stop sequence finished so switch to requested state
        if (requestedState != currentScreenState) 
        {
            currentScreenState = requestedState;
            display.clearDisplay();
            switch (currentScreenState) 
            {
                case SAFE: display.drawBitmap(0, 0, safe_screen, 128, 32, SSD1306_WHITE); break;
                case WARN: display.drawBitmap(0, 0, warn_screen, 128, 32, SSD1306_WHITE); break;
                case STEALTH: display.drawBitmap(0, 0, stealth_screen, 128, 32, SSD1306_WHITE); break;
                case START: display.drawBitmap(0, 0, start_screen, 128, 32, SSD1306_WHITE); break;
                default: break;
            }
            display.display();
        }

        // draw bar + db text for SAFE or WARN
        if (currentScreenState == SAFE || currentScreenState == WARN) 
        {
            drawBar((int16_t)round(smoothedDbm));
            displayUpdateDB(dbValue);
            display.display();
        }
    }
}
