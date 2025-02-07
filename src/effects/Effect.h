// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _EFFECT_H
#define _EFFECT_H

#include "Arduino.h"
#include "ZeroStomp.h"

class Effect
{
public:
    Effect(int16_t mix = MAX_VALUE(int16_t), uint8_t channels = DEFAULT_CHANNELS) {
        setMix(mix);
        setChannels(channels);
    }

    virtual void setChannels(uint8_t value) {
        _isStereo = value == 2;
    }

    virtual void setMix(int16_t value) {
        _mix = value;
    }

    virtual void process(int32_t *l, int32_t *r = nullptr) = 0;

protected:
    bool _isStereo;
    int16_t _mix;

};

#endif
