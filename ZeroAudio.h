// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_AUDIO_H
#define _ZERO_AUDIO_H

#include "Arduino.h"

// Mix down

#define MIX_DOWN_RANGE_LOW (-28000)
#define MIX_DOWN_RANGE_HIGH (28000)
#define MIX_DOWN_RANGE_SHIFT (16)
#define MIX_DOWN_SCALE(x) (0xfffffff / (32768 * x - MIX_DOWN_RANGE_HIGH))

static int16_t mix_down_sample(int32_t sample, int32_t scale) {
    if (sample < MIX_DOWN_RANGE_LOW) {
        sample = (((sample - MIX_DOWN_RANGE_LOW) * scale) >> MIX_DOWN_RANGE_SHIFT) + MIX_DOWN_RANGE_LOW;
    } else if (sample > MIX_DOWN_RANGE_HIGH) {
        sample = (((sample - MIX_DOWN_RANGE_HIGH) * scale) >> MIX_DOWN_RANGE_SHIFT) + MIX_DOWN_RANGE_HIGH;
    }
    return sample;
};

// LFO & Filter ticking

static float global_rate_scale, global_W_scale;
static uint8_t global_tick;

static void control_tick(uint32_t sample_rate, size_t samples) {
    float recip_sample_rate = 1.0 / sample_rate;
    global_rate_scale = samples * recip_sample_rate; // NOTE: Not used currently
    global_W_scale = (2 * PI) * recip_sample_rate;
    global_tick++; // NOTE: Not used currently
};

#endif
