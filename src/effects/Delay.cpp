// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Delay.h"
#include "ZeroStomp.h"

Delay::Delay(size_t buffer_size, float time, float decay, float mix, size_t sample_rate, uint8_t channels) :
    Effect(mix, channels), _sample_rate(sample_rate) {
    setBufferSize(buffer_size);
    setTime(time);
    setDecay(decay);
};

void Delay::setBufferSize(size_t value) {
    _size = max(value, 1);
    reset();
};

void Delay::setTime(float value) {
    _rate = max((float)_size / _sample_rate / value, 0.001);
};

void Delay::setDecay(float value) {
    _decay = min(max(value, 0.0), 1.0);
};

void Delay::process(float *l, float *r) {
    *l = processChannel(*l, 0);
    if (_isStereo) {
        *r = processChannel(*r, 1);
    }
    _pos = fmod(_pos + _rate, _size);
};

float Delay::processChannel(float sample, uint8_t channel) {
    size_t start = (size_t)_pos, end = (size_t)(_pos + _rate);

    // Get current position in buffer before updating buffer
    float output = _buffer[start + _size * channel];

    float echo;
    size_t index;
    for (size_t i = start; i < end; i++) {
        // Determine buffer index
        index = (i % _size) + _size * channel;

        // Get echo value from buffer
        echo = _buffer[index];

        // Apply decay to echo and add current sample
        echo = echo * _decay + sample;

        // Apply dynamic range compression
        echo = mixDown(echo);

        // Update echo buffer with hard clip
        _buffer[index] = min(max(echo, -1.0), 1.0);
    }

    // Mix initial echo value with dry signal and return
    return applyMix(sample, output, _mix);
};

void Delay::reset() {
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    _buffer = (float *)malloc(_size * (_isStereo ? 2 : 1) * sizeof(float));
    memset((void *)_buffer, 0, _size * (_isStereo ? 2 : 1) * sizeof(float));
    _pos = 0.0;
};
