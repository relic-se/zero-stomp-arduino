// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_REVERB_H
#define _ZERO_REVERB_H

#include "Arduino.h"
#include "config.h" // For DEFAULT_CHANNELS

#include "freeverb/revmodel.hpp"

typedef enum {
    NORMAL = 0,
    FREEZE = 1
} ReverbMode;

class ZeroReverb
{

public:
    ZeroReverb(ReverbMode mode = NORMAL, float room_size = initialroom, float damping = initialdamp, float width = initialwidth, float mix = 1.0, uint8_t channels = DEFAULT_CHANNELS);

    void setMode(ReverbMode value);
    void setRoomSize(float value);
    void setDamping(float value);
    void setWidth(float value);
    void setMix(float value);
    void setChannels(uint8_t value);

    void process(int32_t *l, int32_t *r);

private:
    revmodel _model;

};

#endif
