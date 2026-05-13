// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "Gate.h"

Gate::Gate(float threshold, float attack, float decay, size_t sample_rate, uint8_t channels) :
    envelope(threshold, threshold, ENVELOPE_DEFAULT_RISE, ENVELOPE_DEFAULT_FALL, channels),
    _sampleRate(sample_rate) {
    setAttackTime(attack);
    setDecayTime(decay);
};

void Gate::setThreshold(float value) {
    envelope.setAttackLevel(value);
    envelope.setReleaseLevel(value);
};

void Gate::setAttackTime(float value) {
    _attack_rate = (int16_t)(MAX_LEVEL / (value * _sampleRate));
};

void Gate::setDecayTime(float value) {
    _decay_rate = (size_t)(MAX_LEVEL / (value * _sampleRate));
};

bool Gate::isActive() {
    return !envelope.isActive();
};

int16_t Gate::getLevel() {
    return (int16_t)_level;
};

void Gate::applyScale(bool value) {
    _scale = value;
};

void Gate::process(int32_t *l, int32_t *r) {
    // Update envelope and set timer if state changed
    envelope.process(l, r);

    if (envelope.isActive() && _level < MAX_LEVEL) {
        _level = min(_level + _attack_rate, MAX_LEVEL);
    } else if (!envelope.isActive() && _level > 0) {
        _level = max(_level - _decay_rate, 0);
    }

    if (_scale) {
        *l = scale(*l, (int16_t)_level);
        *r = scale(*r, (int16_t)_level);
    }
};
