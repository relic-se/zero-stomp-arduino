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
#define BITS_PER_SAMPLE 16 // BUG: 24 and 32 bit modes cause distortion
#define DEFAULT_CHANNELS 2
#define DEFAULT_BUFFER_SIZE 512
#define MAX_LEVEL ((1 << (BITS_PER_SAMPLE - 1)) - 1)

// I2C

#define PIN_I2C_SDA 6
#define PIN_I2C_SCL 7

#define I2C_WIRE Wire1
#define I2C_SPEED 1000000 // fast mode plus

// Switch

#define PIN_LED 8
#define PIN_SWITCH 9

// Display

#define PIN_DISPLAY_RESET 10
#define PIN_DISPLAY_DC 11
#define PIN_DISPLAY_CS 13
#define PIN_DISPLAY_SCK 14 // Not required when using hardware SPI
#define PIN_DISPLAY_TX 15 // Not required when using hardware SPI

#define DISPLAY_SPI SPI1
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// ADC

#define PIN_ADC_0 26
#define PIN_ADC_1 27
#define PIN_ADC_2 28
#define PIN_ADC_EXPR 29

#define ADC_BITS 12

// Program

#define NUM_DMA_BUFFERS 6
#define CONTROL_RATE 20 // hz
#define SWITCH_DURATION 400 // ms

#include "Arduino.h"
#include "display.h"

#include <SparkFun_WM8960_Arduino_Library.h> 
#include <I2S.h>
#include <Adafruit_SSD1306.h>

class ZeroStomp
{

public:
    ZeroStomp();
    bool begin();

    bool setSampleRate(size_t value);
    bool setChannels(uint8_t value);
    bool setBufferSize(size_t value);

    void setMix(float value);
    void setLevel(float value);

    float getValue(uint8_t index);
    float getExpressionValue();

    bool setTitle(const String &s, bool update = true);
    bool setTitle(const char c[], bool update = true);

    bool setLabel(uint8_t index, const String &s, bool update = true);
    bool setLabel(uint8_t index, const char c[], bool update = true);

    void update();

protected:
    bool updateSampleRate();
    bool updateBitsPerSample();

    bool updateMix();
    bool updateLevel();

    bool prepareTitle(size_t len);
    bool prepareKnobLabel(uint8_t index, size_t len);

    bool drawKnob(uint8_t index);

private:
    WM8960 _codec;
    I2S _i2s;
    size_t _sample_rate;
    bool _isStereo;
    uint8_t _bits_per_sample;
    bool _active = false;
    bool _running = false;

    // Full DAC and Headphone output by default
    float _mix = 1.0, _level = 1.0;

    uint32_t *_buffer;
    size_t _buffer_size, _control_timer = 0;

    Adafruit_SSD1306 _display;
    uint16_t _adc[KNOB_COUNT + 1];
    uint16_t _knob[KNOB_COUNT];

};

// User Functions

void updateControl(size_t samples);
void updateAudio(float *l, float *r);

// Global Objects and Arduino Functions

extern ZeroStomp zeroStomp;

extern void loop();

#ifndef SINGLE_CORE
extern bool core1_separate_stack;
extern void setup1();
extern void loop1();
#endif

// Global Helper Functions

static float dbToLinear(float value) {
    return exp(value * 0.11512925464970228420089957273422);
};

static float mixDown(float sample, uint8_t count = 2, float range = 0.85) {
    float scale = (1.0 / (count - range));
    if (sample < -range) {
        sample = ((sample + range) * scale) - range;
    } else if (sample > range) {
        sample = ((sample - range) * scale) + range;
    }
    return sample;
};

static float applyMix(float dry, float wet, float mix) {
    return mixDown(
        dry * (mix <= 0.5 ? 1.0 : ((1.0 - mix) * 2.0))
        + wet * (mix >= 0.5 ? 1.0 : (mix * 2.0))
    );
};

static float applyLinearMix(float dry, float wet, float mix) {
    return mixDown(dry * (1.0 - mix) + wet * mix);
};

extern float global_rate_scale, global_W_scale;
extern uint8_t global_tick;
static void control_tick(size_t sample_rate, size_t samples) {
    float recip_sample_rate = 1.0 / (float)sample_rate;
    global_rate_scale = (float)samples * recip_sample_rate;
    global_W_scale = (2.0 * PI) * recip_sample_rate;
    global_tick++;
};

#endif
