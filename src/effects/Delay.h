// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_DELAY_H
#define _ZERO_DELAY_H

#include "Arduino.h"

#include "ZeroStomp.h"

#define DELAY_SHIFT (8)

class Delay
{

public:
    Delay(size_t buffer_size, float time = 0.1, uint16_t decay = VOLUME_MIN, uint16_t mix = MIX_WET, size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    void setBufferSize(size_t value);
    void setTime(float value);
    void setDecay(uint16_t value);
    void setMix(uint16_t value);

    void process(int32_t *l, int32_t *r);

protected:
    void reset();
    int32_t processChannel(int32_t sample, uint8_t channel);

private:
    sample_t *_buffer;
    size_t _size, _sample_rate, _pos;
    uint32_t _rate;
    uint16_t _decay, _mix;
    bool _isStereo;

};

#endif
