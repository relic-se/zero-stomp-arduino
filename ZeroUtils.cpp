// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroUtils.h"

float global_rate_scale = 0.0, global_W_scale = 0.0;
uint8_t global_tick = 0;

void control_tick(uint32_t sample_rate, size_t samples) {
    float recip_sample_rate = 1.0 / (float)sample_rate;
    global_rate_scale = samples * recip_sample_rate; // NOTE: Not used currently
    global_W_scale = (2 * PI) * recip_sample_rate;
    global_tick++; // NOTE: Not used currently
};
