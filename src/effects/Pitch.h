// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_CHORUS_H
#define _ZERO_CHORUS_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"

#define PITCH_DEFAULT_WINDOW (DEFAULT_SAMPLE_RATE * 0.02) // 20ms
#define PITCH_DEFAULT_OVERLAP (PITCH_DEFAULT_WINDOW * 0.1) // 2ms
#define PITCH_SHIFT (8)

class Pitch : public Effect
{

public:
    Pitch(size_t window = PITCH_DEFAULT_WINDOW, size_t overlap = PITCH_DEFAULT_OVERLAP, int16_t mix = MAX_VALUE(int16_t), uint8_t channels = DEFAULT_CHANNELS);

    void setShift(float value); // semitones
    void setWindow(size_t value);
    void setOverlap(size_t value);
    void setChannels(uint8_t value) override;

    void process(int32_t *l, int32_t *r = nullptr);

protected:
    int32_t processChannel(int32_t sample, uint8_t channel);
    void reset();

private:
    float _shift;
    size_t _window, _write;
    size_t _read, _rate; // << PITCH_SHIFT
    sample_t *_buffer;
    size_t _overlapSize, _overlapIndex;
    sample_t *_overlapBuffer;

};

#endif
