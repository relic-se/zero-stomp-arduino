// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_ENVELOPE_H
#define _ZERO_ENVELOPE_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"

#define ENVELOPE_DEFAULT_RISE (0.01)
#define ENVELOPE_DEFAULT_FALL (0.0001)

class Envelope : public Effect
{

public:
    Envelope(float rise = ENVELOPE_DEFAULT_RISE, float fall = ENVELOPE_DEFAULT_FALL, uint8_t channels = DEFAULT_CHANNELS);

    void setRise(float value);
    void setFall(float value);

    void process(int32_t *l, int32_t *r = nullptr);
    float get();
    sample_t get_scaled();

protected:
    void reset();

private:
    int16_t _rise, _fall;
    sample_t _accum;

};

#endif
