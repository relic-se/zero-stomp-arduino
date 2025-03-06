// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _DETECT_H
#define _DETECT_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"

#define FFT_SPEED_OVER_PRECISION
#define FFT_SQRT_APPROXIMATION
#include <arduinoFFT.h>

#define DEFAULT_FFT_SIZE (1024)
#define FFT_WINDOWING_FACTORS (1)

#define LOG2 (0.301)
#define LOG2_A4 (8.781) // log(440, 2)

// NOTE: Comment this out to use experimental frequency detection algorithm
#define DETECT_DEFAULT_ANALYZE

#ifndef DETECT_DEFAULT_ANALYZE
#define DETECT_THRESHOLD (0.5)
#define DETECT_MAX_AREAS (8)
#define DETECT_OFFSET (0.0)

typedef struct {
    size_t start, stop, strength;
} DetectArea;
#endif

class Detect : public Effect
{

public:
    Detect(size_t size = DEFAULT_FFT_SIZE, size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    float getFrequency();
    int getNoteNum();
    void getNoteName(char *buffer, size_t size);

    void process(int32_t *l, int32_t *r = nullptr);
    bool ready();
    void flush();

    #ifndef DETECT_DEFAULT_ANALYZE
    float getMaxFrequency();
    float getMinFrequency();
    #endif

protected:
    bool recompute();
    void computeFFT();

    #ifndef DETECT_DEFAULT_ANALYZE
    void analyze();
    float determineAreaFrequency(DetectArea area);
    float frequencyFromIndex(float index);
    #endif

private:
    ArduinoFFT<float> *_fft;
    float *_real;
    float *_imag;
    size_t _size, _sampleRate;
    size_t _pos = 0;
    float _freq = 0.0;

    int _notenum;
    const char *_note_names[12] = {
        "A", "A#/Bb", "B", "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab"
    };

    #ifndef DETECT_DEFAULT_ANALYZE
    float *_data;
    size_t _data_size;
    float _freq_max, _freq_min;

    DetectArea *_areas;
    size_t _num_areas;
    #endif
    
};

#endif
