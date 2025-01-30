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
    Effect(float mix = 1.0, uint8_t channels = DEFAULT_CHANNELS) {
        setMix(mix);
        setChannels(channels);
    }

    virtual void setChannels(uint8_t value) {
        _isStereo = value == 2;
    }

    virtual void setMix(float value) {
        _mix = min(max(value, 0.0), 1.0);
    }

    virtual void process(float *l, float *r) = 0;

protected:
    bool _isStereo;
    float _mix;

};

#endif
