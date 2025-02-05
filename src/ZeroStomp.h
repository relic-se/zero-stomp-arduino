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
#define CONTROL_RATE 16 // hz
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

    bool setSampleRate(uint32_t value);
    bool setChannels(uint8_t value);
    bool setBitsPerSample(uint8_t value);
    bool setBufferSize(size_t value);

    void setMix(uint8_t value);
    void setLevel(uint8_t value);

    uint16_t getValue(uint8_t index);
    uint16_t getExpressionValue();

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
    uint32_t _sample_rate;
    bool _isStereo;
    uint8_t _bits_per_sample;
    bool _active = false;
    bool _running = false;

    // Full DAC and Headphone output by default
    uint8_t _mix = 255;
    uint8_t _level = 255;

    uint32_t *_buffer;
    size_t _buffer_size;
    uint32_t _control_timer = 0;

    Adafruit_SSD1306 _display;
    uint16_t _adc[KNOB_COUNT + 1];
    uint16_t _knob[KNOB_COUNT];

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

#endif
