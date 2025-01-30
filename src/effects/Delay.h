// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _DELAY_H
#define _DELAY_H

#include "Arduino.h"

#include "effects/Effect.h"
#include "ZeroStomp.h"

class Delay : public Effect
{

public:
    Delay(size_t buffer_size, float time = 0.1, float decay = 0.0, float mix = 1.0, size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    void setBufferSize(size_t value);
    void setTime(float value);
    void setDecay(float value);

    void process(float *l, float *r);

protected:
    float processChannel(float sample, uint8_t channel);
    void reset();

private:
    float *_buffer;
    size_t _size, _sample_rate;
    float _pos, _rate, _decay;

};

#endif
