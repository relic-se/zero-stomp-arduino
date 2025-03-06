// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Detect.h"

// TODO: Allow reset using `_fft->setArrays`
// NOTE: Maybe use Adafruit Zero FFT library

Detect::Detect(size_t size, size_t sample_rate, uint8_t channels) :
    Effect(0, channels),
    _size(size), _sampleRate(sample_rate) {

    // TODO: Check if size is power of 2

    size_t buffer_size = _size * (_isStereo + 1) * sizeof(float);
    _real = (float *)malloc(buffer_size);
    _imag = (float *)malloc(buffer_size);

    _fft = new ArduinoFFT<float>(_real, _imag, size, _sampleRate, FFT_WINDOWING_FACTORS);
};

void Detect::process(int32_t *l, int32_t *r) {
    if (_pos >= _size) return;

    int32_t sample = *l;
    if (_isStereo) {
        sample += *r;
        sample = mixDown(sample, MIX_DOWN_SCALE(2));
    }

    _real[_pos++] = convert(sample);
};

void Detect::flush() {
    _pos = 0;
};

float Detect::getFrequency() {
    if (_pos >= _size) {
        _fft->windowing(FFTWindow::Hamming, FFTDirection::Forward);
        _fft->compute(FFTDirection::Forward);
        _fft->complexToMagnitude();
        _freq = _fft->majorPeak();
        _pos = 0;
    }
    return _freq;
};
