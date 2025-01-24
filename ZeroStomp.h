// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_STOMP_H
#define _ZERO_STOMP_H

#include "Arduino.h"
#include <SparkFun_WM8960_Arduino_Library.h> 
#include <I2S.h>
#include "config.h"

class ZeroStomp
{

public:
    ZeroStomp(uint32_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS, uint8_t bits_per_sample = DEFAULT_BITS_PER_SAMPLE, size_t buffer_size = DEFAULT_BUFFER_SIZE);
    bool begin();

    bool setSampleRate(uint32_t value);
    bool setChannels(uint8_t value);
    bool setBitsPerSample(uint8_t value);
    bool setBufferSize(size_t value);

    void setMix(uint8_t value);
    void setLevel(uint8_t value);

    int16_t getAdcValue(uint8_t index);

    void update();

protected:
    bool updateSampleRate();
    bool updateBitsPerSample();

    bool updateMix();
    bool updateLevel();

    int getAdcPin(uint8_t index);

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
    size_t _control_timer = 0;

};

// User Functions
void updateAudio(int32_t *l, int32_t *r);
void updateControl();

#endif
