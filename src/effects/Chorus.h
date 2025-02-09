// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_CHORUS_H
#define _ZERO_CHORUS_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"

#define CHORUS_SHIFT (2)

class Chorus : public Effect
{

public:
    Chorus(float max_time = 0.1, float time = 0.1, uint8_t voices = 2, int16_t mix = MAX_VALUE(int16_t), size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    void setMaxTime(float value);
    void setTime(float value);
    void setVoices(uint8_t value);
    void setSampleRate(size_t value);
    void setChannels(uint8_t value) override;

    void process(int32_t *l, int32_t *r = nullptr);

protected:
    int32_t processChannel(int32_t sample, uint8_t channel);
    void updateOffset();
    void updateStep();
    void reset();

private:
    sample_t *_buffer;
    float _max_time, _time;
    size_t _size, _sample_rate, _pos;
    size_t _offset, _current_offset, _step; // << CHORUS_SHIFT
    uint8_t _voices;
    int32_t _scale; // For mixDown

};

#endif
