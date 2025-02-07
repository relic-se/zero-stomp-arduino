// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "LFO.h"

#define ALMOST_ONE (32767.0 / 32768.0)

int16_t lfoWaveformTriangle(size_t index) {
    switch (index / (LFO_WAVEFORM_LEN / 4)) {
        case 0:
        default:
            return index * LFO_WAVEFORM_TRIANGLE_ACCUM;
        case 1:
        case 2:
            return (index - (LFO_WAVEFORM_LEN / 2)) * -LFO_WAVEFORM_TRIANGLE_ACCUM;
        case 3:
            return (index - LFO_WAVEFORM_LEN) * LFO_WAVEFORM_TRIANGLE_ACCUM;
    }
};

int16_t lfoWaveformSquare(size_t index) {
    return (index < LFO_WAVEFORM_LEN / 2) ? LFO_WAVEFORM_MAX : -LFO_WAVEFORM_MAX;
};

int16_t lfoWaveformSine(size_t index) {
    return (int16_t)(sin((float)index * 2 * PI / LFO_WAVEFORM_LEN) * LFO_WAVEFORM_MAX);
};

int16_t lfoWaveformSaw(size_t index) {
    return (index - (LFO_WAVEFORM_LEN / 2)) * -LFO_WAVEFORM_MAX / (LFO_WAVEFORM_LEN / 2);
};

LFO::LFO(float rate, float scale, float offset, float phase_offset) :
    _rate(rate),
    _scale(scale),
    _offset(offset),
    _phase_offset(phase_offset) {

    // Default to triangle wave
    setWaveform(lfoWaveformTriangle);

};

void LFO::setOnce(bool value) {
    _once = value;
};

void LFO::setInterpolate(bool value) {
    _interpolate = value;
};

void LFO::setRate(float value) {
    _rate = value;
};

void LFO::setScale(float value) {
    _scale = value;
};

void LFO::setOffset(float value) {
    _offset = value;
};

void LFO::setPhaseOffset(float value) {
    _phase_offset = value;
};

void LFO::setWaveform(LfoWaveform waveform) {
    for (size_t i = 0; i < LFO_WAVEFORM_LEN; i++) {
        _waveform[i] = waveform(i);
    }
};

float LFO::get() {
    tick();
    return _value;
};

float LFO::get_limited(float lo, float hi) {
    tick();
    if (_value < lo) {
        return lo;
    }
    if (_value > hi) {
        return hi;
    }
    return _value;
};

int32_t LFO::get_scaled(float lo, float hi) {
    return convert(get_limited(lo, hi), SAMPLE_SHIFT);
};

void LFO::retrigger() {
    _accum = 0.0;
};

void LFO::tick() {
    if (_tick == global_tick) {
        return;
    }
    _tick = global_tick;

    float rate = _rate * global_rate_scale;
    float accum = _accum + rate + _phase_offset;

    if (_once) {
        if (rate > 0) {
            if (accum > ALMOST_ONE) {
                accum = ALMOST_ONE;
            }
        } else if (rate < 0 && accum < 0.0) {
            accum = 0.0;
        }
    } else {
        accum = accum - floor(accum);
    }
    _accum = accum - _phase_offset;

    float scaled_accum = accum * (LFO_WAVEFORM_LEN - _once);
    size_t index = (size_t)floor(scaled_accum);
    if (index >= LFO_WAVEFORM_LEN) {
        index = _once ? (LFO_WAVEFORM_LEN - 1) : (index % LFO_WAVEFORM_LEN);
    }

    _value = (float)_waveform[index];

    if (_interpolate) {
        float frac = scaled_accum - index;

        size_t next_index = index + 1;
        if (next_index == LFO_WAVEFORM_LEN) {
            next_index = _once ? index : 0;
        }
        _value = _value * (1.0 - frac) + _waveform[next_index] * frac;
    }

    _value = ldexp(_value, -15) * _scale + _offset;
};
