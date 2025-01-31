// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _DELAY_H
#define _DELAY_H

#include "Arduino.h"

#include "effects/Effect.h"
#include "ZeroStomp.h"

#define DELAY_USE_FLOAT
#define DELAY_POS_SHIFT (8)

class Delay : public Effect
{

public:
    Delay(size_t buffer_size, float time = 0.1, float decay = 0.0, float mix = 1.0, size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    void setBufferSize(size_t value);
    void setSampleRate(size_t value);
    void setChannels(uint8_t value) override;

    void setTime(float value);
    void setDecay(float value);

    void process(float *l, float *r);

protected:
    float processChannel(float sample, uint8_t channel);
    void reset();

private:
    #ifdef DELAY_USE_FLOAT
    float *_buffer;
    #else
    int16_t *_buffer;
    #endif
    size_t _size, _sample_rate;
    size_t _pos, _rate;
    #ifdef DELAY_USE_FLOAT
    float _decay;
    #else
    int16_t _decay;
    #endif

};

#endif
