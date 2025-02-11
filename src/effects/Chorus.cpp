// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Chorus.h"

Chorus::Chorus(float max_time, float time, uint8_t voices, int16_t mix, size_t sample_rate, uint8_t channels) :
    Effect(mix, channels) {
    setSampleRate(sample_rate);
    setMaxTime(max_time);
    setTime(time);
    setVoices(voices);

    _current_offset = _offset;
    updateStep();

    reset();
};

void Chorus::setMaxTime(float value) {
    _max_time = max(value, 0.001);
    _size = max(_max_time * _sampleRate, 1);
    updateOffset();
    if (_buffer) reset();
};

void Chorus::setTime(float value) {
    _time = min(max(value, 0.001), _max_time);
    updateOffset();
};

void Chorus::setVoices(uint8_t value) {
    _voices = max(value, 1);
    _scale = MIX_DOWN_SCALE(_voices);
    updateStep();
};

void Chorus::setSampleRate(size_t value) {
    _sampleRate = value;
    if (_buffer) reset();
};

void Chorus::setChannels(uint8_t value) {
    Effect::setChannels(value);
    if (_buffer) reset();
};

void Chorus::process(int32_t *l, int32_t *r) {
    *l = processChannel(*l, 0);
    if (_isStereo) {
        *r = processChannel(*r, 1);
    }
    
    if (++_pos >= _size) _pos = 0;

    if (_offset != _current_offset) {
        if (_offset > _current_offset) {
            _current_offset++;
        } else if (_offset < _current_offset) {
            _current_offset--;
        }
        updateStep();
    }

};

int32_t Chorus::processChannel(int32_t sample, uint8_t channel) {
    // Update current position of buffer
    _buffer[_pos + _size * channel] = (sample_t)clip(sample);

    int32_t output = 0;
    int32_t pos = (int32_t)_pos << CHORUS_SHIFT;
    for (uint8_t i = 0; i < _voices; i++) {
        pos -= _step;
        if (pos < 0) pos += _size << CHORUS_SHIFT;

        output += _buffer[(pos >> CHORUS_SHIFT) + _size * channel];
    }

    // Apply dynamic range compression
    if (_voices > 1) output = mixDown(output, _scale);

    // Mix with dry signal and return
    return applyMix<int16_t>(sample, output, _mix);
};

void Chorus::updateOffset() {
    _offset = min(max(_time * _sampleRate * (1 << CHORUS_SHIFT), 1), (_size - 1) << CHORUS_SHIFT);
};

void Chorus::updateStep() {
    _step = max(_current_offset / _voices, 1);
};

void Chorus::reset() {
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    _buffer = (sample_t *)malloc(_size * (_isStereo + 1) * sizeof(sample_t));
    memset((void *)_buffer, 0, _size * (_isStereo + 1) * sizeof(sample_t));
    _pos = 0;
};
