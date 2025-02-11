// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Delay.h"

Delay::Delay(size_t buffer_size, float time, int16_t decay, int16_t mix, size_t sample_rate, uint8_t channels) :
    Effect(mix, channels) {
    setBufferSize(buffer_size);
    setSampleRate(sample_rate);
    setTime(time);
    setDecay(decay);
};

void Delay::setBufferSize(size_t value) {
    _size = max(value, 1);
    reset();
};

void Delay::setSampleRate(size_t value) {
    // Adjust rate if necessary
    if (_rate && _sample_rate) {
        _rate = _rate * _sample_rate / value;
    }
    _sample_rate = value;
};

void Delay::setTime(float value) {
    _rate = (uint32_t)max(
        (float)_size * (1 << DELAY_SHIFT) / _sample_rate / value,
        1.0
    );
};

void Delay::setDecay(int16_t value) {
    _decay = value;
};

void Delay::setChannels(uint8_t value) {
    Effect::setChannels(value);
    reset();
};

void Delay::process(int32_t *l, int32_t *r) {
    *l = processChannel(*l, 0);
    if (_isStereo) {
        *r = processChannel(*r, 1);
    }
    _pos = (_pos + _rate) % (_size << DELAY_SHIFT);
};

int32_t Delay::processChannel(int32_t sample, uint8_t channel) {
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
        echo = scale<int16_t>(echo, _decay) + sample;

        // Apply dynamic range compression
        echo = mixDown(echo, MIX_DOWN_SCALE(2));

        // Update echo buffer with hard clip
        _buffer[index] = (sample_t)clip(echo);
    }

    // Mix initial echo value with dry signal and return
    return applyMix<int16_t>(sample, output, _mix);
};

void Delay::reset() {
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    _buffer = (sample_t *)malloc(_size * (_isStereo ? 2 : 1) * sizeof(sample_t));
    memset((void *)_buffer, 0, _size * (_isStereo ? 2 : 1) * sizeof(sample_t));
    _pos = 0;
};
