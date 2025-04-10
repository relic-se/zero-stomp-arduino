// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/SpringReverb.h"

SpringReverb::SpringReverb(float frequency, int16_t mix, size_t sample_rate, uint8_t channels) :
    Effect(mix, channels),
    _filter(FilterMode::LOW_PASS, frequency, DEFAULT_FILTER_Q, MAX_VALUE(int16_t), channels) {
    setSampleRate(sample_rate);

    reset();
};

void SpringReverb::setFrequency(float value) {
    _filter.frequency = value;
    _filter.update();
};

void SpringReverb::setSampleRate(size_t value) {
    _sample_rate = value;
    if (_buffer) reset();
};

void SpringReverb::setChannels(uint8_t value) {
    Effect::setChannels(value);
    _filter.setChannels(value);
    if (_buffer) reset();
};

void SpringReverb::reset() {
    // TODO
};

void SpringReverb::process(int32_t *l, int32_t *r) {
    _filter.process(l, r);
};
