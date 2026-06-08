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
#define DB_TEXT_X 57    // top left corner of box for the speedometer version
#define DB_TEXT_Y 6

// speed text properties
#define SPEED_TEXT_X 2    // moved far left to clear space for a large number
#define SPEED_TEXT_Y 10   // centered vertically for size 2 text

// configurable for db scaling
int dbMin = -60;
int dbMax = 5;

// thresholds
int warnDB = -30;   // dbm to go to warn
int stopDB = -20;   // dbm to go to stop

String currentVersion = "v1.3s";    // s meaning speed

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


// initialize display
void displayInit() 
{
    // pins b6 and b7 are used for i2c
    Wire.begin();
    Wire.setClock(100000);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
    {
        return; // cannot init display
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
    display.printf("%+3d", dbValue);    // removed dB text to save space
}

// update display with large speed value
void displayUpdateSpeed(int16_t speedValue) 
{
    int speedY = SPEED_TEXT_Y;
    if (speedY < 0) speedY = 0;
    if (speedY > SCREEN_HEIGHT - 16) speedY = SCREEN_HEIGHT - 16;

    // wipe the entire left diagnostic container cleanly
    display.fillRect(0, 0, 52, SCREEN_HEIGHT, SSD1306_BLACK);

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(SPEED_TEXT_X, speedY);
    
    // displays large real-time rolling mph speed configuration
    display.printf("%3d", speedValue);
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

void updateTextAndBar(int16_t dbValue, int16_t speedValue) 
{
    // Smooth bar logic...
    if (dbValue > smoothedDbm) {
        smoothedDbm = dbValue;
    } else {
        smoothedDbm += (dbValue - smoothedDbm) * barFallFactor;
    }

    ScreenState requestedState;
    if (dbValue >= stopDB) requestedState = STOP;
    else if (dbValue >= warnDB) requestedState = WARN;
    else requestedState = SAFE;

    if (requestedState != currentScreenState) 
    {
        currentScreenState = requestedState;
        display.clearDisplay();
        switch (currentScreenState) 
        {
            case SAFE: display.drawBitmap(0, 0, safe_screen, 128, 32, SSD1306_WHITE); break;
            case WARN: display.drawBitmap(0, 0, warn_screen, 128, 32, SSD1306_WHITE); break;
            case STOP: display.drawBitmap(0, 0, stop_screen, 128, 32, SSD1306_WHITE); break;
            case STEALTH: display.drawBitmap(0, 0, stealth_screen, 128, 32, SSD1306_WHITE); break;
            case START: display.drawBitmap(0, 0, start_screen, 128, 32, SSD1306_WHITE); break;
            default: break;
        }
    }

    if (currentScreenState == SAFE || currentScreenState == WARN || currentScreenState == STOP) 
    {
        drawBar((int16_t)round(smoothedDbm));
        displayUpdateDB(dbValue);
        displayUpdateSpeed(speedValue);
    }

    // --- CRASH RECOVERY IMPLEMENTATION ---
    // Wire.endTransmission() returns 0 if successful. 
    // display.display() returns nothing, but we can verify if the bus is alive.
    display.display(); 

    Wire.beginTransmission(OLED_ADDR);
    byte error = Wire.endTransmission();

    if (error != 0) 
    {
        // The display has dropped off the bus or frozen! Force a reconnection.
        Wire.begin(); 
        display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
        
        // Force screen redraw on next loop by resetting state
        currentScreenState = START; 
    }
}