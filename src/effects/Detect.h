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

#define DEFAULT_FFT_SIZE (DEFAULT_BUFFER_SIZE)
#define FFT_WINDOWING_FACTORS (1)

class Detect : public Effect
{

public:
    Detect(size_t size = DEFAULT_FFT_SIZE, size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    float getFrequency();

    void process(int32_t *l, int32_t *r = nullptr);
    void flush();

protected:
    void reset();

private:
    ArduinoFFT<float> *_fft;
    float *_real;
    float *_imag;
    size_t _size, _sampleRate;
    size_t _pos = 0;
    float _freq = 0.0;
    
};

#endif
