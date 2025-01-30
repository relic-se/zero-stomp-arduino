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
    setChannels(channels);
};

void ZeroReverb::setMode(ReverbMode mode) {
    _model.setmode(mode == FREEZE ? freezemode : 0.0);
};

void ZeroReverb::setRoomSize(float value) {
    _model.setroomsize(value);
};

void ZeroReverb::setDamping(float value) {
    _model.setdamp(value);
};

void ZeroReverb::setWidth(float value) {
    _model.setwidth(value);
};

void ZeroReverb::setMix(float value) {
    value *= 2.0;
    _model.setdry(min(2.0 - value, 1.0));
    _model.setwet(min(value, 1.0));
};

void ZeroReverb::setChannels(uint8_t value) {
    _model.setstereo(value == 2);
};

void ZeroReverb::process(int32_t *l, int32_t *r) {
    float fl = sampleToFloat(*l), fr = 0;
    if (_model.getstereo()) {
        fr = sampleToFloat(*r);
    }
    _model.processreplace(&fl, &fr, &fl, &fr, 1, 0);
    *l = sampleFromFloat(fl);
    if (_model.getstereo()) {
        *r = sampleFromFloat(fr);
    }
};
