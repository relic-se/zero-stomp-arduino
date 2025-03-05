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
    _attack_time = (size_t)(value * _sampleRate);
};

void Gate::setDecayTime(float value) {
    _decay_time = (size_t)(value * _sampleRate);
};

bool Gate::isActive() {
    return !envelope.isActive();
};

void Gate::process(int32_t *l, int32_t *r) {
    // Update envelope and set timer if state changed
    envelope.process(l, r);
    if (envelope.didAttack()) {
        _timer = _attack_time;
    } else if (envelope.didRelease()) {
        _timer = _decay_time;
    }

    if (!isActive() && !_timer) {
        // Gate active
        *l = *r = 0;
    } else if (isActive() && _timer) {
        // Gate rising
        *l = *l * (_attack_time - _timer) / _attack_time;
        *r = *r * (_attack_time - _timer) / _attack_time;
    } else if (!isActive() && _timer) {
        // Gate falling
        *l = *l * _timer / _decay_time;
        *r = *r * _timer / _decay_time;
    }

    if (_timer) _timer--;
};
