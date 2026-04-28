// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_STOMP_H
#define _ZERO_STOMP_H

// UART

#define PIN_UART_TX 0
#define PIN_UART_RX 1

// I2S

#define PIN_I2S_BCLK 2
#define PIN_I2S_LRCLK 3
#define PIN_I2S_DOUT 4
#define PIN_I2S_DIN 5

#define DEFAULT_SAMPLE_RATE 48000
#define BITS_PER_SAMPLE 16
#define DEFAULT_CHANNELS 2
#define DEFAULT_BUFFER_SIZE 512

#define MIN_LEVEL 0
#define MAX_LEVEL ((1 << (BITS_PER_SAMPLE - 1)) - 1)

// Codec

#define USE_PGA

// I2C

#define PIN_I2C_SDA 6
#define PIN_I2C_SCL 7

#define I2C_WIRE Wire1
#define I2C_SPEED 1000000 // fast mode plus

// Switch

#define PIN_LED 8
#define PIN_SWITCH 9
#define LED_FREQUENCY 100000
#define MAX_LED 65535

// Display

#define PIN_DISPLAY_RESET 10
#define PIN_DISPLAY_DC 11
#define PIN_DISPLAY_CS 13
#define PIN_DISPLAY_SCK 14 // Not required when using hardware SPI
#define PIN_DISPLAY_TX 15 // Not required when using hardware SPI

#define DISPLAY_SPI SPI1

// ADC

#define PIN_ADC_0 26
#define PIN_ADC_1 27
#define PIN_ADC_2 28
#define PIN_ADC_EXPR 29

#define ADC_BITS 12

// Program

#define NUM_DMA_BUFFERS 6
#define CONTROL_RATE 16 // hz
#define SWITCH_DURATION 400 // ms
#define MAX_CONTROLS 16

#include "Arduino.h"
#include "display.h"
#include "controls/Control.h"
#include <SparkFun_WM8960_Arduino_Library.h> 
#include <I2S.h>
#include <Adafruit_SSD1306.h>

#if BITS_PER_SAMPLE == 16
typedef int16_t sample_t;
#else
typedef int32_t sample_t;
#endif

typedef void (*BypassChangeCallback)(bool);
typedef void (*ClickCallback)(uint8_t);
class ZeroStomp
{

public:
    ZeroStomp();
    bool begin();

    bool setSampleRate(uint32_t value);
    uint32_t getSampleRate();
    bool setChannels(uint8_t value);
    uint8_t getChannels();
    bool setBitsPerSample(uint8_t value);
    uint8_t getBitsPerSample();
    bool setBufferSize(size_t value);
    size_t getBufferSize();

    void setMix(uint8_t value);
    uint8_t getMix();
    void setLevel(uint8_t value);
    uint8_t getLevel();

    int getExpression();
    int getExpression(int max_value);
    int getExpression(int min_value, int max_value);

    float getExpressionFloat();
    float getExpressionFloat(float max_value);
    float getExpressionFloat(float min_value, float max_value);

    void setTitle(const String &s, bool update = true);
    void setTitle(const char c[], bool update = true);

    void update();
    void updateControls(size_t samples);

    // Switch Functions
    bool isBypassed();
    void setBypassChange(BypassChangeCallback cb = nullptr);
    void setClick(ClickCallback cb = nullptr);

    uint16_t getLed();
    void setLed(uint16_t value);

    bool addControl(Control *control);
    bool addControls(int count, ...);

    size_t getPageCount();
    size_t getPage();
    size_t getPageControlCount();

    void previousPage(bool update = true);
    void nextPage(bool update = true);

    Adafruit_SSD1306 display;

protected:
    bool updateSampleRate();
    bool updateBitsPerSample();

    bool updateMix();
    bool updateLevel();

    bool drawTitle(bool update = true);

    bool clearPage(bool update = true);
    bool drawPage(bool update = true);
    bool drawPageTitle(bool update = true);

    void redraw();

private:
    WM8960 _codec;
    I2S _i2s;
    size_t _sampleRate;
    bool _isStereo;
    uint8_t _bits_per_sample;
    bool _active = false;
    bool _running = false;

    bool _led_control = true;
    uint16_t _led_value = 0;

    bool _switch_value = false;
    unsigned long _switch_millis = 0;
    uint8_t _switch_count = 0;
    BypassChangeCallback _bypass_change_cb = nullptr;
    ClickCallback _click_cb = nullptr;

    // Full DAC and Headphone output by default
    uint8_t _mix = 255;
    uint8_t _level = 255;

    size_t _buffer_size;
    size_t _control_timer = 0;

    const char *_title;
    size_t _title_len = 0;

    Control *_controls[MAX_CONTROLS];
    size_t _num_controls = 0;
    size_t _page = 0;

    int _adc_expr = -1;

};

// User Functions
void updateControl(size_t samples);
void updateAudio(int16_t *l, int16_t *r);

extern ZeroStomp zeroStomp;

extern void loop();

#ifndef SINGLE_CORE
extern bool core1_separate_stack;
extern void setup1();
extern void loop1();
#endif

extern float global_rate_scale, global_W_scale;
extern uint8_t global_tick;
static void control_tick(size_t sample_rate, size_t samples) {
    float recip_sampleRate = 1.0 / (float)sample_rate;
    global_rate_scale = (float)samples * recip_sampleRate;
    global_W_scale = (2.0 * PI) * recip_sampleRate;
    global_tick++;
};

// Global Helper Functions

#define SHIFT(T) (sizeof(T) * 8 - 1)
#define MAX_VALUE(T) ((1 << SHIFT(T)) - 1)
#define MID_VALUE(T) (1 << (SHIFT(T) - 1))

#define USHIFT(T) (sizeof(T) * 8)
#define UMAX_VALUE(T) ((1 << USHIFT(T)) - 1)
#define UMID_VALUE(T) (1 << (USHIFT(T) - 1))

#define SAMPLE_SHIFT (SHIFT(int16_t))
#define SAMPLE_MAX_VALUE (MAX_VALUE(int16_t))
#define SAMPLE_MID_VALUE (MID_VALUE(int16_t))

static float dbToLinear(float value) {
    return exp(value * 0.11512925464970228420089957273422);
};

static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

static int mapControl(int value, int min_value, int max_value) {
    return map(min(max(value, CONTROL_MIN), CONTROL_MAX), CONTROL_MIN, CONTROL_MAX, min_value, max_value);
};

static int mapControl(int value, int max_value) {
    return mapControl(value, 0, max_value);
};

static float mapControlFloat(int value, float min_value, float max_value) {
    return mapFloat(min(max(value, CONTROL_MIN), CONTROL_MAX), CONTROL_MIN, CONTROL_MAX, min_value, max_value);
};

static float mapControlFloat(int value, float max_value) {
    return mapControlFloat(value, 0.0, max_value);
};

inline float convertToFloat(int32_t sample) {
    return ldexp(sample, -SAMPLE_SHIFT);
};

inline int16_t convertToInt(float sample) {
    return round(ldexp(sample, SAMPLE_SHIFT));
};

inline int16_t convertToInt(float value, int8_t shift) {
    return round(ldexp(value, shift));
};

inline float clip(float sample, float level = 1.0) {
    return min(max(sample, -level), level);
};

inline int16_t clip(int32_t sample, int16_t level = -1) {
    if (level < 0) level = SAMPLE_MAX_VALUE;
    return min(max(sample, -level), level);
};

inline int16_t scale(int16_t sample, int16_t value) {
    return (sample * (int32_t)value) >> SAMPLE_SHIFT;
};

inline int16_t scale(int16_t sample, int16_t value, int8_t shift) {
    return (sample * (int32_t)value) >> shift;
};

#define MIX_DOWN_RANGE (28000)
#define MIX_DOWN_SCALE(x) (0xfffffff / (32768 * x - MIX_DOWN_RANGE))

static int16_t mixDown(int32_t sample, int32_t scale = MIX_DOWN_SCALE(2)) {
    if (sample < -MIX_DOWN_RANGE) {
        sample = (((sample + MIX_DOWN_RANGE) * scale) >> (sizeof(int16_t) * 8)) - MIX_DOWN_RANGE;
    } else if (sample > MIX_DOWN_RANGE) {
        sample = (((sample - MIX_DOWN_RANGE) * scale) >> (sizeof(int16_t) * 8)) + MIX_DOWN_RANGE;
    }
    return sample;
};

static int16_t applyMix(int16_t dry, int16_t wet, int16_t mix) {
    return mixDown(
        scale(dry, mix <= SAMPLE_MID_VALUE ? SAMPLE_MAX_VALUE : ((SAMPLE_MAX_VALUE - mix) << 1))
        + scale(wet, mix >= SAMPLE_MID_VALUE ? SAMPLE_MAX_VALUE : mix << 1),
        MIX_DOWN_SCALE(2)
    );
};

static int16_t applyLinearMix(int16_t dry, int16_t wet, int16_t mix) {
    return mixDown(
        scale(dry, SAMPLE_MAX_VALUE - mix)
        + scale(wet, mix),
        MIX_DOWN_SCALE(2)
    );
};

#endif
