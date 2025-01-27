// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroAudio.h"

int16_t mixDown(int32_t sample, int32_t scale) {
    if (sample < MIX_DOWN_RANGE_LOW) {
        sample = (((sample - MIX_DOWN_RANGE_LOW) * scale) >> MIX_DOWN_RANGE_SHIFT) + MIX_DOWN_RANGE_LOW;
    } else if (sample > MIX_DOWN_RANGE_HIGH) {
        sample = (((sample - MIX_DOWN_RANGE_HIGH) * scale) >> MIX_DOWN_RANGE_SHIFT) + MIX_DOWN_RANGE_HIGH;
    }
    return sample;
};
