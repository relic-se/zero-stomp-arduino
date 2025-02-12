// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "Envelope.h"

Envelope::Envelope(float rise, float fall, uint8_t channels) :
    Effect(0, channels) {
    setRise(rise);
    setFall(fall);
    reset();
};

void Envelope::setRise(float value) {
    _rise = (int16_t)(value * MAX_VALUE(int16_t));
};

void Envelope::setFall(float value) {
    _fall = (int16_t)(value * MAX_VALUE(int16_t));
};

void Envelope::process(int32_t *l, int32_t *r = nullptr) {
    int32_t sample = *l;
    if (_isStereo && r != nullptr) {
        sample += *r;
        // No need for dynamic compression
    }

    int16_t mix = (_accum < sample ? _rise : _fall);
    _accum = applyLinearMix<int16_t>(_accum, sample, mix);
};

float Envelope::get() {
    return convert(_accum); // Convert to float
};

sample_t Envelope::get_scaled() {
    return _accum; // Already scaled
};

void Envelope::reset() {
    _accum = 0;
};
