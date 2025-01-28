// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_LFO_H
#define _ZERO_LFO_H

#include "Arduino.h"

extern float global_rate_scale;
extern uint8_t global_tick;

#define LFO_WAVEFORM_LEN 256
#define LFO_WAVEFORM_MAX 32767

#define LFO_WAVEFORM_TRIANGLE_ACCUM (LFO_WAVEFORM_MAX / (LFO_WAVEFORM_LEN / 4))

typedef int16_t (*LfoWaveform)(size_t);

int16_t lfoWaveformTriangle(size_t index);
int16_t lfoWaveformSquare(size_t index);
int16_t lfoWaveformSine(size_t index);
int16_t lfoWaveformSaw(size_t index);

class ZeroLFO
{

public:
    ZeroLFO(float rate = 1.0, float scale = 1.0, float offset = 0.0, float phase_offset = 0.0);

    void setOnce(bool value);
    void setInterpolate(bool value);

    void setRate(float value);
    void setScale(float value);
    void setOffset(float value);
    void setPhaseOffset(float value);

    void setWaveform(LfoWaveform waveform);

    float get();
    float get_limited(float lo, float hi);
    int32_t get_scaled(float lo, float hi);

    void retrigger();

protected:
    void tick();

private:
    bool _once = false, _interpolate = true;
    float _rate, _scale, _offset, _phase_offset;

    float _accum, _value;
    uint8_t _tick = (1 << 8) - 1;

    int16_t _waveform[LFO_WAVEFORM_LEN];

};

#endif
