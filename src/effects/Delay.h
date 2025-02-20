// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _DELAY_H
#define _DELAY_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"

#define DELAY_SHIFT (8)

class Delay : public Effect
{

public:
    Delay(size_t buffer_size, float time = 0.1, int16_t decay = MIN_LEVEL, int16_t mix = MAX_VALUE(int16_t), size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    void setBufferSize(size_t value);
    void setSampleRate(size_t value);
    void setChannels(uint8_t value) override;

    void setTime(float value);
    void setDecay(int16_t value);

    void process(int32_t *l, int32_t *r = nullptr);

protected:
    int32_t processChannel(int32_t sample, uint8_t channel);
    void reset();

private:
    sample_t *_buffer;
    size_t _size, _sampleRate;
    size_t _pos, _rate;
    int16_t _decay;

};

#endif
