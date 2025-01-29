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
    size_t start = _pos >> DELAY_SHIFT, end = (_pos + _rate) >> DELAY_SHIFT;

    // Get current position in buffer before updating buffer
    int16_t output = _buffer[start + _size * channel];

    int32_t echo;
    size_t index;
    for (size_t i = start; i < end; i++) {
        // Determine buffer index
        index = (i % _size) + _size * channel;

        // Get echo value from buffer
        echo = (int32_t)_buffer[index];

        // Apply decay to echo and add current sample
        echo = applyVolume(echo, _decay) + sample;

        // Apply dynamic range compression
        echo = mixDown(echo, MIX_DOWN_SCALE(2));

        // Update echo buffer with hard clip
        _buffer[index] = (int16_t)min(max(echo, -HARD_CLIP), HARD_CLIP);
    }

    // Mix initial echo value with dry signal and return
    return applyMix(sample, output, _mix);
};
