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

class MultiTapDelay : public Effect
{

public:
    MultiTapDelay(
        float max_time,
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
    void setTaps(size_t count, const Tap **taps);

    void process(int32_t *l, int32_t *r = nullptr);

protected:
    int32_t processChannel(int32_t sample, uint8_t channel);
    void reset();
    void updateSize();
    void updateTapOffsets();

private:
    float _max_time, _time;
    size_t _sample_rate;
    int16_t _decay;

    sample_t *_buffer;
    size_t _max_size, _size, _pos;

    const Tap **_taps = nullptr;
    size_t _tap_count = 0;
    size_t *_tap_offsets;
    int32_t _scale; // For mixDown

};

#endif
