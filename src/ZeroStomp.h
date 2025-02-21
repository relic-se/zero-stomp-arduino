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
    bool setChannels(uint8_t value);
    bool setBitsPerSample(uint8_t value);
    bool setBufferSize(size_t value);

    void setMix(uint8_t value);
    void setLevel(uint8_t value);

    int getExpression();
    int getExpression(int max_value);
    int getExpression(int min_value, int max_value);

    float getExpressionFloat();
    float getExpressionFloat(float max_value);
    float getExpressionFloat(float min_value, float max_value);

    void setTitle(const String &s, bool update = true);
    void setTitle(const char c[], bool update = true);

    void update();
    void updateControls(uint32_t samples);

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
    uint32_t _sampleRate;
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

    uint32_t *_buffer;
    size_t _buffer_size;
    uint32_t _control_timer = 0;

    Adafruit_SSD1306 _display;

    const char *_title;
    size_t _title_len = 0;

    Control *_controls[MAX_CONTROLS];
    size_t _num_controls = 0;
    size_t _page = 0;

    int _adc_expr = -1;

};

// User Functions
void updateControl(uint32_t samples);
void updateAudio(int32_t *l, int32_t *r);

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

#define SAMPLE_SHIFT (BITS_PER_SAMPLE - 1)
#define SAMPLE_MAX_VALUE ((1 << SAMPLE_SHIFT) - 1)

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

inline float convert(int32_t sample) {
    return ldexp(sample, -SAMPLE_SHIFT);
};

inline int32_t convert(float sample) {
    return (int32_t)round(ldexp(sample, SAMPLE_SHIFT));
};

template <typename T>
inline T convert(float value) {
    return (T)round(ldexp(value, SHIFT(T)));
};

inline int32_t convert(float value, int8_t shift) {
    return (int32_t)round(ldexp(value, shift));
};

inline float clip(float sample, float level = 1.0) {
    return min(max(sample, -level), level);
};

inline int32_t clip(int32_t sample, int32_t level = -1) {
    if (level < 0) level = SAMPLE_MAX_VALUE;
    return min(max(sample, -level), level);
};

template <typename T>
inline int32_t scale(int32_t sample, T value) {
    return (sample * (int32_t)value) >> SHIFT(T);
};

inline int32_t scale(int32_t sample, int32_t value, int8_t shift) {
    return (sample * value) >> shift;
};

#define MIX_DOWN_RANGE_F (0.85)
#define MIX_DOWN_SCALE_F(x) (1.0 / (x - MIX_DOWN_RANGE_F))

static float mixDown(float sample, float scale = MIX_DOWN_SCALE_F(2.0)) {
    if (sample < -MIX_DOWN_RANGE_F) {
        sample = ((sample + MIX_DOWN_RANGE_F) * scale) - MIX_DOWN_RANGE_F;
    } else if (sample > MIX_DOWN_RANGE_F) {
        sample = ((sample - MIX_DOWN_RANGE_F) * scale) + MIX_DOWN_RANGE_F;
    }
    return sample;
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

static float applyMix(float dry, float wet, float mix) {
    if (mix >= 1.0) return wet;
    if (mix <= 0.0) return dry;
    return mixDown(
        (float)(dry * (mix <= 0.5 ? 1.0 : ((1.0 - mix) * 2.0))
        + wet * (mix >= 0.5 ? 1.0 : (mix * 2.0)))
    );
};

template <typename T>
static int32_t applyMix(int32_t dry, int32_t wet, T mix) {
    return mixDown(
        scale<T>(dry, mix <= MID_VALUE(T) ? MAX_VALUE(T) : ((MAX_VALUE(T) - mix) << 1))
        + scale<T>(wet, mix >= MID_VALUE(T) ? MAX_VALUE(T) : mix << 1),
        MIX_DOWN_SCALE(2)
    );
};

static float applyLinearMix(float dry, float wet, float mix) {
    if (mix >= 1.0) return wet;
    if (mix <= 0.0) return dry;
    return mixDown((float)(dry * (1.0 - mix) + wet * mix));
};

template <typename T>
static int32_t applyLinearMix(int32_t dry, int32_t wet, T mix) {
    return mixDown(
        scale<T>(dry, MAX_VALUE(T) - mix)
        + scale<T>(wet, mix),
        MIX_DOWN_SCALE(2)
    );
};

#endif
