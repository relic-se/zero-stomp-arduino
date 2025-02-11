// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Pitch.h"

Pitch::Pitch(size_t window, size_t overlap, int16_t mix, uint8_t channels) : Effect(mix, channels) {
    setWindow(window);
    setOverlap(overlap);
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

void Pitch::setOverlap(size_t value) {
    _overlapSize = min(value, _window - 1);
    reset();
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

    // Increment write buffer pointer
    if (++_write >= _window) _write = 0;

    // Increment overlap buffer pointer
    if (_overlapSize && ++_overlapIndex >= _overlapSize) _overlapIndex = 0;

    // Increment read buffer by rate
    _read += _rate;
    if (_read >= _window << PITCH_SHIFT) _read -= _window << PITCH_SHIFT;
};

int32_t Pitch::processChannel(int32_t sample, uint8_t channel) {
    if (_overlapSize) {
        // Pull last sample from overlap and store in buffer
        _buffer[_write + _window * channel] = _overlapBuffer[_overlapIndex + _overlapSize * channel];

        // Save current sample in overlap
        _overlapBuffer[_overlapIndex + _overlapSize * channel] = (sample_t)clip(sample);
    } else {
        // Write sample to buffer
        _buffer[_write + _window * channel] = (sample_t)clip(sample);
    }

    // Determine how far we are into the overlap
    size_t readIndex = _read >> PITCH_SHIFT;
    size_t readOverlapOffset = readIndex + _window * (readIndex < _write) - _write;

    // Read sample from buffer
    int32_t output = (int32_t)_buffer[readIndex + _window * channel];
    
    // Check if we're within the overlap range
    if (readOverlapOffset > 0 && readOverlapOffset <= _overlapSize) {
        // Blend buffer sample with overlap sample
        output *= (int32_t)readOverlapOffset; // Apply blend volume to buffer sample
        output += (int32_t)_overlapBuffer[((_overlapIndex + readOverlapOffset) % _overlapSize) + _overlapSize * channel] * (int32_t)(_overlapSize - readOverlapOffset); // Add overlap with blend volume
        output /= (int32_t)_overlapSize; // Scale down
    }

    // Mix with dry signal and return
    return applyMix<int16_t>(sample, output, _mix);
};

void Pitch::reset() {
    // Reset circular buffer
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    _buffer = (sample_t *)malloc(_window * (_isStereo + 1) * sizeof(sample_t));
    memset((void *)_buffer, 0, _window * (_isStereo + 1) * sizeof(sample_t));

    // Reset last write buffer
    if (_overlapBuffer) {
        free (_overlapBuffer);
        _overlapBuffer = nullptr;
    }
    if (_overlapSize) {
        _overlapBuffer = (sample_t *)malloc(_overlapSize * (_isStereo + 1) * sizeof(sample_t));
        memset((void *)_overlapBuffer, 0, _overlapSize * (_isStereo + 1) * sizeof(sample_t));
    }

    // Reset buffer pointers
    _write = _overlapIndex = _read = 0;
};
