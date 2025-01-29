// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroDelay.h"

ZeroDelay::ZeroDelay(size_t buffer_size, float time, uint16_t decay, uint16_t mix, size_t sample_rate, uint8_t channels) :
    _sample_rate(sample_rate),
    _decay(decay),
    _mix(mix) {
    _isStereo = channels == 2;
    setBufferSize(buffer_size);
    setTime(time);
};

void ZeroDelay::reset() {
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    _buffer = (int16_t *)malloc(_size * (_isStereo ? 2 : 1) * sizeof(int16_t));
    memset((void *)_buffer, 0, _size * (_isStereo ? 2 : 1) * sizeof(int16_t));
    _pos = 0;
};

void ZeroDelay::setBufferSize(size_t value) {
    _size = max(value, 1);
    reset();
};

void ZeroDelay::setTime(float value) {
    _rate = (uint32_t)max(
        (float)_size * (1 << DELAY_SHIFT) / _sample_rate / value,
        1.0
    );
};

void ZeroDelay::setDecay(uint16_t value) {
    _decay = value;
};

void ZeroDelay::setMix(uint16_t value) {
    _mix = value;
};

void ZeroDelay::process(int32_t *l, int32_t *r) {
    *l = processChannel(*l, 0);
    if (_isStereo) {
        *r = processChannel(*r, 1);
    }
    _pos = (_pos + _rate) % (_size << DELAY_SHIFT);
};

int32_t ZeroDelay::processChannel(int32_t sample, uint8_t channel) {
    // Get current echo value from buffer
    int32_t echo = (int32_t)_buffer[(_pos >> DELAY_SHIFT) * (channel + 1)];

    // Apply decay to echo and add current sample
    int32_t mix = applyVolume(echo, _decay) + sample;

    // Apply dynamic range compression
    mix = mixDown(mix, MIX_DOWN_SCALE(2));

    // Update echo buffer
    for (size_t i = _pos >> DELAY_SHIFT; i < (_pos + _rate) >> DELAY_SHIFT; i++) {
        _buffer[(i % _size) * (channel + 1)] = (int16_t)mix;
    }

    // Mix original echo value with dry signal and return
    return applyMix(sample, echo, _mix);
};
