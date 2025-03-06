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
    
    _real = (float *)malloc(_size * sizeof(float));
    _imag = (float *)malloc(_size * sizeof(float));

    _fft = new ArduinoFFT<float>(_real, _imag, _size, _sampleRate, FFT_WINDOWING_FACTORS);
    
    #ifndef DETECT_DEFAULT_ANALYZE
    _freq_max = (float)_sampleRate / 2.0; // Nyquist
    _freq_min = (float)_sampleRate / _size;

    _data_size = (_size / 2) - 2;
    _data = (float *)malloc(_data_size * sizeof(float));

    _areas = new DetectArea[DETECT_MAX_AREAS];
    #endif
};

void Detect::process(int32_t *l, int32_t *r) {
    if (_pos >= _size) return;

    int32_t sample = *l;
    if (_isStereo) {
        sample += *r;
        sample = mixDown(sample, MIX_DOWN_SCALE(2));
    }

    _real[_pos] = convert(sample);
    _imag[_pos++] = 0.0;
};

void Detect::flush() {
    _pos = 0;
};

float Detect::getFrequency() {
    recompute();
    return _freq;
};

int Detect::getNoteNum() {
    if (recompute()) {
        _notenum = round(12.0 * (log(_freq) / LOG2 - LOG2_A4) + 69.0);
    }
    return _notenum;
};

const char *Detect::getNoteName() {
    int notenum = getNoteNum();
    snprintf(
        _note_name,
        sizeof(_note_name),
        "%s%d",
        _note_names[(notenum - 21) % 12],
        (notenum - 12) / 12
    );
    return (const char *)&_note_name;
};

bool Detect::needsRecompute() {
    return _pos >= _size;
};

bool Detect::recompute() {
    if (!needsRecompute()) return false;
    computeFFT();
    #ifdef DETECT_DEFAULT_ANALYZE
    _freq = _fft->majorPeak();
    #else
    analyze(); // BUG: Not ready!
    #endif
    flush();
    return true;
};

void Detect::computeFFT() {
    _fft->dcRemoval();
    _fft->windowing(FFTWindow::Hamming, FFTDirection::Forward);
    _fft->compute(FFTDirection::Forward);
    _fft->complexToMagnitude();
};

#ifndef DETECT_DEFAULT_ANALYZE

void Detect::analyze() {
    size_t i;

    // Copy data, ignore top half of fft and first and last elements
    memcpy((void *)_data, (void *)(_real + 1), _data_size * sizeof(float));

    // Get minimum and maximum values
    float minval = INFINITY, maxval = 0.0;
    for (i = 0; i < _data_size; i++) {
        if (_data[i] < minval) minval = _data[i];
        if (_data[i] > maxval) maxval = _data[i];
    }

    // Calculate relative threshold
    float threshold = (maxval - minval) * DETECT_THRESHOLD + minval;

    // Apply threshold to data
    for (i = 0; i < _data_size; i++) {
        _data[i] = _data[i] > threshold ? _data[i] : 0.0;
    }

    // Determine frequency areas
    _num_areas = 0;
    DetectArea *area = nullptr;
    for (i = 0; i < _data_size; i++) {
        if (_data[i] > 0.0) {
            if (area == nullptr) {
                area = new DetectArea;
                area->start = i;
                area->stop = i;
                area->strength = 0;
            }
            area->stop += 1;
            area->strength += _data[i];
        }
        if (area != nullptr && (_data[i] <= 0.0 || i == _data_size - 1)) {
            // NOTE: May want to average strength by area size?
            _areas[_num_areas++] = *area;
            area = nullptr;
            if (_num_areas >= DETECT_MAX_AREAS) break;
        }
    }

    // No frequency areas identified
    if (!_num_areas) {
        _freq = 0.0;
        return;
    }

    // Find strongest area
    size_t maxareaindex = 0;
    for (i = 1; i < _num_areas; i++) {
        if (_areas[i].strength < _areas[maxareaindex].strength) continue;
        maxareaindex = i;
    }

    // Calculate center frequency of area
    _freq = determineAreaFrequency(_areas[maxareaindex]);
};

float Detect::determineAreaFrequency(DetectArea area) {
    // Calculate center index using weighted mean
    float sum = 0.0, sum_dist = 0.0;
    for (size_t i = area.start; i < area.stop; i++) {
        sum += _data[i];
        sum_dist += _data[i] * frequencyFromIndex(i);
    }
    return frequencyFromIndex(sum_dist / sum);
};

float Detect::frequencyFromIndex(float index) {
    // Index is linearly distributed
    return (_freq_max - _freq_min) * (index / (_data_size - 1)) + _freq_min + DETECT_OFFSET;
};

float Detect::getMaxFrequency() {
    return _freq_max;
};

float Detect::getMinFrequency() {
    return _freq_min;
};

#endif
