// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroUtils.h"

float dbToLinear(float value) {
    return exp(value * 0.11512925464970228420089957273422);
};

int16_t mixDown(int32_t sample, int32_t scale) {
    if (sample < MIX_DOWN_RANGE_LOW) {
        sample = (((sample - MIX_DOWN_RANGE_LOW) * scale) >> MIX_DOWN_RANGE_SHIFT) + MIX_DOWN_RANGE_LOW;
    } else if (sample > MIX_DOWN_RANGE_HIGH) {
        sample = (((sample - MIX_DOWN_RANGE_HIGH) * scale) >> MIX_DOWN_RANGE_SHIFT) + MIX_DOWN_RANGE_HIGH;
    }
    return sample;
};

float global_rate_scale = 0.0, global_W_scale = 0.0;
uint8_t global_tick = 0;

void control_tick(uint32_t sample_rate, size_t samples) {
    float recip_sample_rate = 1.0 / (float)sample_rate;
    global_rate_scale = (float)samples * recip_sample_rate;
    global_W_scale = (2.0 * PI) * recip_sample_rate;
    global_tick++;
};
