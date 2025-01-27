// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_AUDIO_H
#define _ZERO_AUDIO_H

#include "Arduino.h"

#define MIX_DOWN_RANGE_LOW (-28000)
#define MIX_DOWN_RANGE_HIGH (28000)
#define MIX_DOWN_RANGE_SHIFT (16)
#define MIX_DOWN_SCALE(x) (0xfffffff / (32768 * x - MIX_DOWN_RANGE_HIGH))

int16_t mixDown(int32_t sample, int32_t scale);

#endif
