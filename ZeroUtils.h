// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_UTILS_H
#define _ZERO_UTILS_H

#include "Arduino.h"

static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

// LFO & Filter ticking

extern float global_rate_scale, global_W_scale;
extern uint8_t global_tick;

void control_tick(uint32_t sample_rate, size_t samples);

#endif
