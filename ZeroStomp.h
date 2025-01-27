// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_STOMP_H
#define _ZERO_STOMP_H

#include "Arduino.h"
#include "config.h"
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

    uint8_t *_buffer;
    size_t _buffer_size;
    uint32_t _control_timer = 0;

    Adafruit_SSD1306 _display;
    uint16_t _adc[KNOB_COUNT + 1];
    uint16_t _knob[KNOB_COUNT];

};

// User Functions
void updateAudio(int32_t *l, int32_t *r);
void updateControl(uint32_t samples);

extern ZeroStomp zeroStomp;

extern void loop();

#ifndef SINGLE_CORE
extern bool core1_separate_stack;
extern void setup1();
extern void loop1();
#endif

#endif
