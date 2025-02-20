// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _FILTER_H
#define _FILTER_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"

extern float global_W_scale;

#define BIQUAD_SHIFT (15)

typedef enum {
    LOW_PASS = 0,
    HIGH_PASS = 1,
    BAND_PASS = 2,
    NOTCH = 3
} FilterMode;

#define FILTER_MODES 4

#define FOUR_OVER_PI (4 / PI)
static void fast_sincos(float theta, float *s, float *c) {
    float x = (theta * FOUR_OVER_PI) - 1;
    float x2 = x * x, x3 = x2 * x, x4 = x2 * x2, x5 = x2 * x3;
    float c0 = 0.70708592,
          c1x = -0.55535724 * x,
          c2x2 = -0.21798592 * x2,
          c3x3 = 0.05707685 * x3,
          c4x4 = 0.0109 * x4,
          c5x5 = -0.00171961 * x5;

    float evens = c4x4 + c2x2 + c0, odds = c5x5 + c3x3 + c1x;
    *s = evens - odds;
    *c = evens + odds;
};

static bool float_equal_or_update(float *cached, float value) {
    // uses memcmp to avoid error about equality float comparison
    if (memcmp(cached, &value, sizeof(float))) {
        *cached = value;
        return false;
    }
    return true;
};

class Filter : public Effect
{

public:
    Filter(FilterMode m = LOW_PASS, float f = 20000.0, float q = 0.7071067811865475, int16_t mix = MAX_VALUE(int16_t), uint8_t channels = DEFAULT_CHANNELS);
    
    FilterMode mode;
    float frequency, Q;

    void update();
    void assign(float a1, float a2, float b0, float b1, float b2);
    
    void process(int32_t *l, int32_t *r = nullptr);
    void reset();

protected:
    int32_t processSample(int32_t input);

private:
    FilterMode _mode;
    float _W0, _Q;
    int32_t _a1, _a2, _b0, _b1, _b2;
    int32_t _x[2], _y[2];

};

#endif
