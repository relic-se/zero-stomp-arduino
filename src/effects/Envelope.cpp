// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "Envelope.h"

Envelope::Envelope(float attack, float release, float rise, float fall, uint8_t channels) :
    Effect(0, channels) {
    setAttackLevel(attack);
    setReleaseLevel(release);
    setRise(rise);
    setFall(fall);
    reset();
};

void Envelope::setAttackLevel(float value) {
    _attack = convertToInt(value);
};

void Envelope::setAttackCallback(EnvelopeAttackCallback cb) {
    _attack_cb = cb;
};

void Envelope::setReleaseLevel(float value) {
    _release = convertToInt(value);
};

void Envelope::setReleaseCallback(EnvelopeAttackCallback cb) {
    _release_cb = cb;
};

bool Envelope::isActive() {
    return _active;
};

bool Envelope::didAttack() {
    return _did_attack;
};

bool Envelope::didRelease() {
    return _did_release;
};

void Envelope::setRise(float value) {
    _rise = convertToInt(value);
};

void Envelope::setFall(float value) {
    _fall = convertToInt(value);
};

void Envelope::process(int16_t *l, int16_t *r) {
    _did_attack = _did_release = false;

    int16_t sample = *l;
    if (_isStereo && r != nullptr) {
        sample += *r;
        // No need for dynamic compression
    }

    int16_t mix = (_accum < sample ? _rise : _fall);
    _accum = applyLinearMix(_accum, max(sample, 0), mix);

    if (!_active && _accum > _attack) {
        _active = true;
        _did_attack = true;
        if (_attack_cb != nullptr) (*_attack_cb)();
    } else if (_active && _accum < _release) {
        _active = false;
        _did_release = true;
        if (_release_cb != nullptr) (*_release_cb)();
    }
};

float Envelope::get() {
    return convertToFloat(_accum); // Convert to float
};

int32_t Envelope::get_scaled() {
    return _accum; // Already scaled
};

void Envelope::reset() {
    _accum = 0;
    _active = false;
};
