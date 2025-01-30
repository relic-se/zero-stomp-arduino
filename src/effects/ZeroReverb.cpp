// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/ZeroReverb.h"
#include "ZeroUtils.h"

ZeroReverb::ZeroReverb(ReverbMode mode, float room_size, float damping, float width, float mix, uint8_t channels) {
    setMode(mode);
    setRoomSize(room_size);
    setDamping(damping);
    setWidth(width);
    setMix(mix);
    _isStereo = channels == 2;
};

void ZeroReverb::setMode(ReverbMode mode) {
    model.setmode(mode == FREEZE ? freezemode : 0.0);
};

void ZeroReverb::setRoomSize(float value) {
    model.setroomsize(value);
};

void ZeroReverb::setDamping(float value) {
    model.setdamp(value);
};

void ZeroReverb::setWidth(float value) {
    model.setwidth(value);
};

void ZeroReverb::setMix(float value) {
    value *= 2.0;
    model.setdry(min(2.0 - value, 1.0));
    model.setwet(min(value, 1.0));
};

void ZeroReverb::process(int32_t *l, int32_t *r) {
    float fl = sampleToFloat(*l), fr = 0;
    if (_isStereo) {
        fr = sampleToFloat(*r);
    }
    model.processreplace(&fl, &fr, &fl, &fr, 1, 0);
    *l = sampleFromFloat(fl);
    if (_isStereo) {
        *r = sampleFromFloat(fr);
    }
};
