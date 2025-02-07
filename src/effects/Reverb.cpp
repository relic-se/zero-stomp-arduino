// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Reverb.h"

Reverb::Reverb(ReverbMode mode, float room_size, float damping, float width, int16_t mix, size_t sample_rate, uint8_t channels) :
    Effect(mix, channels) {
    setMode(mode);
    setRoomSize(room_size);
    setDamping(damping);
    setWidth(width);
    setMix(mix);
    setChannels(channels);
};

void Reverb::setMode(ReverbMode mode) {
    _model.setmode(mode == FREEZE ? freezemode : 0.0);
};

void Reverb::setRoomSize(float value) {
    _model.setroomsize(value);
};

void Reverb::setDamping(float value) {
    _model.setdamp(value);
};

void Reverb::setWidth(float value) {
    _model.setwidth(value);
};

void Reverb::setMix(int16_t value) {
    Effect::setMix(value);
    float mix = convert<int16_t>(value);
    mix *= 2.0;
    _model.setdry(min(2.0 - mix, 1.0));
    _model.setwet(min(mix, 1.0));
};

void Reverb::setSampleRate(size_t value) {
    //_model.setsamplerate(value); // BUG: Not implemented
};

void Reverb::setChannels(uint8_t value) {
    Effect::setChannels(value);
    _model.setstereo(value == 2);
};

void Reverb::process(int32_t *l, int32_t *r) {
    float fl = convert(*l), fr = 0;
    if (_model.getstereo()) {
        fr = convert(*r);
    }
    _model.processreplace(&fl, &fr, &fl, &fr, 1, 0);
    *l = convert(fl);
    if (_model.getstereo()) {
        *r = convert(fr);
    }
};
