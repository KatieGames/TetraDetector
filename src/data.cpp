#include "data.h"
#include <cstring>
#include <algorithm>

// --- Config ---
static constexpr float adc_ref_voltage = 3.3f;
static constexpr uint16_t adc_max = 1023;
static constexpr float ad_slope = 0.018f;      // mV/dB
static constexpr float ad_intercept = -95.0f;  // dBm at 0 V

static constexpr uint8_t median_size = 5;        // optional smoothing

// --- Peak hold config ---
static uint32_t peak_hold_ms = 500;          // hold time in ms
static float peak_decay_per_ms = 0.2f;       // dB/ms decay after hold

// --- State ---
static float voltage_buffer[median_size];
static uint8_t buf_index = 0;
static bool buffer_full = false;

static float last_voltage = 0.0f;
static float last_dbm = -80.0f;
static float peak_dbm = -80.0f;

static uint32_t last_update_ms = 0;
static uint32_t peak_timestamp_ms = 0;   // when peak was last updated

// --- Helpers ---
static float median(float *v, uint8_t n) 
{
    float temp[median_size];
    memcpy(temp, v, n * sizeof(float));

    for (uint8_t i = 1; i < n; i++) 
    {
        float key = temp[i];
        int j = i - 1;
        while (j >= 0 && temp[j] > key) 
        {
            temp[j + 1] = temp[j];
            j--;
        }
        temp[j + 1] = key;
    }

    return temp[n / 2];
}

void dataInit() 
{
    memset(voltage_buffer, 0, sizeof(voltage_buffer));
    last_voltage = 0.0f;
    last_dbm = -80.0f;
    peak_dbm = -80.0f;
    buf_index = 0;
    buffer_full = false;
    last_update_ms = millis();
    peak_timestamp_ms = 0;
}

// add new ADC sample
void dataAddSample(uint16_t adc) 
{
    uint32_t now = millis();
    uint32_t dt = now - last_update_ms;
    last_update_ms = now;

    // adc to voltage
    float v = (adc / (float)adc_max) * adc_ref_voltage;
    last_voltage = v;

    // buffer for median smoothing
    voltage_buffer[buf_index++] = v;
    if (buf_index >= median_size) 
    {
        buf_index = 0;
        buffer_full = true;
    }

    uint8_t count = buffer_full ? median_size : buf_index;

    // median or latest sample
    float v_med = median(voltage_buffer, count); // or just v

    // convert to dbm
    last_dbm = (v_med / ad_slope) + ad_intercept;

    // peak hold
    if (last_dbm > peak_dbm) 
    {
        // rises instantly
        peak_dbm = last_dbm;
        peak_timestamp_ms = now;
    } 
    else 
    {
        // check if still within hold time
        if (now - peak_timestamp_ms > peak_hold_ms) 
        {
            // decay
            peak_dbm -= dt * peak_decay_per_ms;
            if (peak_dbm < last_dbm) peak_dbm = last_dbm;
        }
    }
}

float dataGetDbm() 
{
    return peak_dbm;    // return peak hold value for display
}