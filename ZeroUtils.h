// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_UTILS_H
#define _ZERO_UTILS_H

#include "Arduino.h"

// Float operations

static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

static float sampleToFloat(int32_t value) {
    return (float)value / 32768.0;
};

static int32_t sampleFromFloat(float value) {
    return (int32_t)(value * 32767.0);
};

float dbToLinear(float value);

// Sample mixing

#define MIX_DOWN_RANGE_LOW (-28000)
#define MIX_DOWN_RANGE_HIGH (28000)
#define MIX_DOWN_RANGE_SHIFT (16)
#define MIX_DOWN_SCALE(x) (0xfffffff / (32768 * x - MIX_DOWN_RANGE_HIGH))

int16_t mixDown(int32_t sample, int32_t scale);

// LFO & Filter ticking

extern float global_rate_scale, global_W_scale;
extern uint8_t global_tick;

void control_tick(uint32_t sample_rate, size_t samples);

#endif
