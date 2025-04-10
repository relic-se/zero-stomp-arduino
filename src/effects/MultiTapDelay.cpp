// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/MultiTapDelay.h"

MultiTapDelay::MultiTapDelay(float time, int16_t decay, int16_t mix, size_t sample_rate, uint8_t channels) :
    Effect(mix, channels) {
    setSampleRate(sample_rate);
    setTime(time);
    setDecay(decay);

    reset();
};

void MultiTapDelay::setSampleRate(size_t value) {
    _sample_rate = value;
    if (_buffer) reset();
};

void MultiTapDelay::setChannels(uint8_t value) {
    Effect::setChannels(value);
    if (_buffer) reset();
};

void MultiTapDelay::setTime(float value) {
    _time = value;
    if (_buffer) reset();
};

float MultiTapDelay::getTime() {
    return _time;
};

void MultiTapDelay::setDecay(int16_t value) {
    _decay = value;
};

void MultiTapDelay::setTaps(size_t count, Tap **taps) {
    _taps = taps;
    _tap_count = count;
    _scale = MIX_DOWN_SCALE(_tap_count);
    if (_buffer) updateTapOffsets();
};

void MultiTapDelay::process(int32_t *l, int32_t *r) {
    *l = processChannel(*l, 0);
    if (_isStereo) {
        *r = processChannel(*r, 0);
    }
    if (++_pos >= _size) _pos = 0;
};

int32_t MultiTapDelay::processChannel(int32_t sample, uint8_t channel) {
    // Sum taps based on position and level
    int32_t output = 0;
    size_t pos;
    int32_t echo;
    for (size_t i = 0; i < _tap_count; i++) {
        pos = (_pos + _size - _tap_offsets[i]) % _size;
        echo = _buffer[pos + _size * channel];
        output += scale<int16_t>(echo, _taps[i]->level);
    }

    // Apply dynamic range compression
    if (_tap_count > 1) output = mixDown(output, _scale);

    // Get last value from buffer
    echo = (int32_t)_buffer[_pos + _size * channel];

    // Apply decay to echo and add current sample
    echo = scale<int16_t>(echo, _decay) + sample;

    // Apply dynamic range compression
    echo = mixDown(echo, MIX_DOWN_SCALE(2));

    // Update buffer with hard clip
    _buffer[_pos + _size * channel] = (sample_t)clip(echo);

    // Mix with dry signal and return
    return applyMix<int16_t>(sample, output, _mix);
};

void MultiTapDelay::reset() {
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }

    _size = max(_time * _sample_rate, 1);

    _buffer = (sample_t *)malloc(_size * (_isStereo + 1) * sizeof(sample_t));
    memset((void *)_buffer, 0, _size * (_isStereo + 1) * sizeof(sample_t));

    _pos = 0;
    updateTapOffsets();
};

void MultiTapDelay::updateTapOffsets() {
    if (_tap_offsets) {
        free(_tap_offsets);
        _tap_offsets = nullptr;
    }

    if (!_tap_count || !_taps) return;

    _tap_offsets = (size_t *)malloc(_tap_count * sizeof(size_t));
    for (size_t i = 0; i < _tap_count; i++) {
        _tap_offsets[i] = min(max(_size * _taps[i]->position, 0), _size - 1);
    }
};
