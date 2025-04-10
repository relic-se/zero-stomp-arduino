// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _SPRING_REVERB_H
#define _SPRING_REVERB_H

#include "Arduino.h"
#include "ZeroStomp.h" // For DEFAULT_CHANNELS
#include "effects/Effect.h"

typedef struct tap_t {
    float position; // 0.0 to 1.0
    int16_t level;
} Tap;

const Tap DefaultTap = { 1.0, MAX_LEVEL };
const Tap *DefaultTaps[1] = { &DefaultTap };

class MultiTapDelay : public Effect
{

public:
    MultiTapDelay(
        float time = 0.1,
        int16_t decay = MIN_LEVEL,
        int16_t mix = MAX_VALUE(int16_t),
        size_t sample_rate = DEFAULT_SAMPLE_RATE,
        uint8_t channels = DEFAULT_CHANNELS
    );

    void setSampleRate(size_t value);
    void setChannels(uint8_t value) override;

    void setTime(float value);
    float getTime();
    
    void setDecay(int16_t value);
    void setTaps(size_t count, Tap *taps);

    void process(int32_t *l, int32_t *r = nullptr);

protected:
    int32_t processChannel(int32_t sample, uint8_t channel);
    void reset();
    void updateTapOffsets();

private:
    float _time;
    size_t _sample_rate;
    int16_t _decay;

    sample_t *_buffer;
    size_t _size, _pos;

    Tap **_taps = &DefaultTaps;
    size_t _tap_count = 1;
    size_t _tap_offsets;
    int32_t _scale; // For mixDown

};

#endif
