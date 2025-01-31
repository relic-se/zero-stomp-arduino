// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Delay.h"
#include "ZeroStomp.h"

Delay::Delay(size_t buffer_size, float time, float decay, float mix, size_t sample_rate, uint8_t channels) :
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
    _rate = (size_t)max(
        (float)_size * (1 << DELAY_SHIFT) / _sample_rate / value,
        1.0
    );
};

void Delay::setDecay(float value) {
    value = min(max(value, 0.0), 1.0);
    #ifdef DELAY_USE_FLOAT
    _decay = value;
    #else
    _decay = scale<int16_t>(value);
    #endif
};

void Delay::setChannels(uint8_t value) {
    Effect::setChannels(value);
    reset();
};

void Delay::process(float *l, float *r) {
    *l = processChannel(*l, 0);
    if (_isStereo) {
        *r = processChannel(*r, 1);
    }
    _pos = (_pos + _rate) % (_size << DELAY_SHIFT);
};

float Delay::processChannel(float sample, uint8_t channel) {
    size_t start = _pos >> DELAY_SHIFT, end = (_pos + _rate) >> DELAY_SHIFT;

    // Get current position in buffer before updating buffer
    float output;
    #ifdef DELAY_USE_FLOAT
    output = _buffer[start + _size * channel];
    #else
    output = convert<int16_t>(_buffer[(_pos >> DELAY_SHIFT) + _size * channel]);
    #endif

    #ifdef DELAY_USE_FLOAT
    float echo;
    #else
    int16_t input = convert<int16_t>(sample);
    int32_t echo;
    #endif

    size_t index;
    for (size_t i = start; i < end; i++) {
        // Determine buffer index
        index = (i % _size) + _size * channel;

        // Get echo value from buffer
        echo = _buffer[index];

        // Apply decay to echo and add current sample
        #ifdef DELAY_USE_FLOAT
        echo = echo * _decay + sample;
        #else
        echo = scale<int16_t>(echo, _decay) + input;
        #endif

        // Apply dynamic range compression
        echo = mixDown(echo);

        // Hard clip
        echo = clip(echo);

        // Update echo buffer
        _buffer[index] = echo;
    }

    // Mix initial echo value with dry signal and return
    return applyMix(sample, output, _mix);
};

void Delay::reset() {
    if (_buffer) free(_buffer);
    #ifdef DELAY_USE_FLOAT
    _buffer = (float *)malloc(_size * (_isStereo ? 2 : 1) * sizeof(float));
    #else
    _buffer = (int16_t *)malloc(_size * (_isStereo ? 2 : 1) * sizeof(int16_t));
    #endif
    memset((void *)_buffer, 0, _size * (_isStereo ? 2 : 1) * sizeof(_buffer[0]));
    _pos = 0;
};
