// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Pitch.h"

Pitch::Pitch(size_t window, int16_t mix, uint8_t channels) : Effect(mix, channels) {
    setWindow(window);
    reset();
};

void Pitch::setShift(float value) {
    _shift = value;
    _rate = (size_t)(pow(2, _shift / 12.0) * (1 << PITCH_SHIFT));
};

void Pitch::setWindow(size_t value) {
    _window = max(value, 2);
    if (_buffer) reset();
};

void Pitch::setChannels(uint8_t value) {
    Effect::setChannels(value);
    if (_buffer) reset();
};

void Pitch::process(int32_t *l, int32_t *r) {
    *l = processChannel(*l, 0);
    if (_isStereo) {
        *r = processChannel(*r, 1);
    }
    
    if (++_write >= _window) _write = 0;

    _read += _rate;
    if (_read >= _window << PITCH_SHIFT) _read -= _window << PITCH_SHIFT;
};

int32_t Pitch::processChannel(int32_t sample, uint8_t channel) {
    // Update current position of buffer
    _buffer[_write + _window * channel] = (sample_t)clip(sample);

    size_t index = _read >> PITCH_SHIFT;
    int32_t output = _buffer[index + _window * channel];

    #if PITCH_SHIFT == 8
    // Blend current sample with next sample
    int16_t pos = _read & ((1 << PITCH_SHIFT) - 1);
    if (pos) {
        output = applyMix<int8_t>(
            output,
            _buffer[((index + 1) % _window) + _window * channel],
            (int8_t)pos - (1 << (PITCH_SHIFT - 1))
        );
    }
    #endif

    // Mix with dry signal and return
    return applyMix<int16_t>(sample, output, _mix);
};

void Pitch::reset() {
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    _buffer = (sample_t *)malloc(_window * (_isStereo + 1) * sizeof(sample_t));
    memset((void *)_buffer, 0, _window * (_isStereo + 1) * sizeof(sample_t));
    _write = _read = 0;
};
