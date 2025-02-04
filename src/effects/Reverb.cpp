// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Reverb.h"

Reverb::Reverb(ReverbMode mode, float room_size, float damping, float width, float mix, size_t sample_rate, uint8_t channels) :
    Effect(mix, channels) {
    setMode(mode);
    setRoomSize(room_size);
    setDamping(damping);
    setWidth(width);
    setSampleRate(sample_rate);
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

void Reverb::setMix(float value) {
    Effect::setMix(value);
    value *= 2.0;
    _model.setdry(min(2.0 - value, 1.0));
    _model.setwet(min(value, 1.0));
};

void Reverb::setSampleRate(size_t value) {
    _model.setsamplerate(value);
};

void Reverb::setChannels(uint8_t value) {
    Effect::setChannels(value);
    _model.setstereo(value == 2);
};

void Reverb::process(float *l, float *r) {
    _model.processreplace(l, r, l, r, 1, 0);
};
