// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_UTILS_H
#define _ZERO_UTILS_H

#include "Arduino.h"

#define MAX_LEVEL ((1 << 14) - 1)

// Float operations

static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

static float sampleToFloat(int32_t value) {
    return (float)value / MAX_LEVEL;
};

static int32_t sampleFromFloat(float value) {
    return (int32_t)(value * MAX_LEVEL);
};

float dbToLinear(float value);

// Sample mixing

#define MIX_DOWN_RANGE_LOW (-28000)
#define MIX_DOWN_RANGE_HIGH (28000)
#define MIX_DOWN_RANGE_SHIFT (16)
#define MIX_DOWN_SCALE(x) (0xfffffff / (32768 * x - MIX_DOWN_RANGE_HIGH))

int16_t mixDown(int32_t sample, int32_t scale);

// Volume

#define VOLUME_SHIFT (15)
#define VOLUME_MIN (0)
#define VOLUME_MAX (1 << VOLUME_SHIFT)

static int32_t applyVolume(int32_t sample, uint16_t level) {
    return (sample * (int32_t)level) >> VOLUME_SHIFT;
};

// Mix

#define MIX_DRY (VOLUME_MIN)
#define MIX_MID (VOLUME_MAX >> 1)
#define MIX_WET (VOLUME_MAX)

static int32_t applyMix(int32_t dry, int32_t wet, uint16_t mix) {
    return mixDown(
        applyVolume(dry, mix <= MIX_MID ? VOLUME_MAX : ((MIX_WET - mix) << 1))
        + applyVolume(wet, mix >= MIX_MID ? VOLUME_MAX : mix << 1),
        MIX_DOWN_SCALE(2)
    );
};

static int32_t applyLinearMix(int32_t dry, int32_t wet, uint16_t mix) {
    return mixDown(
        applyVolume(dry, MIX_WET - mix)
        + applyVolume(wet, mix),
        MIX_DOWN_SCALE(2)
    );
};

// LFO & Filter ticking

extern float global_rate_scale, global_W_scale;
extern uint8_t global_tick;

void control_tick(uint32_t sample_rate, size_t samples);

#endif
