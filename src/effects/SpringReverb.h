// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _SPRING_REVERB_H
#define _SPRING_REVERB_H

#include "Arduino.h"
#include "ZeroStomp.h" // For DEFAULT_CHANNELS
#include "effects/Effect.h"
#include "effects/Filter.h"

/*
Resources:
- http://recherche.ircam.fr/pub/dafx11/Papers/39_e.pdf
- https://asp-eurasipjournals.springeropen.com/counter/pdf/10.1155/2011/646134.pdf
- https://github.com/tomas-gajarsky/parametric-spring-reverb/blob/master/parametricSpring.m
*/

class SpringReverb : public Effect
{

public:
    SpringReverb(
        float frequency = 4300.0,
        int16_t mix = MAX_VALUE(int16_t),
        size_t sample_rate = DEFAULT_SAMPLE_RATE,
        uint8_t channels = DEFAULT_CHANNELS
    );

    void setFrequency(float value);
    void setDelay();
    void setSampleRate(size_t value);
    void setChannels(uint8_t value) override;

    void process(int32_t *l, int32_t *r = nullptr);

protected:
    int32_t processChannel(int32_t sample, uint8_t channel);
    void reset();

private:
    Filter _filter;
    size_t _sample_rate;

    sample_t *_buffer;
    size_t _buffer_pos;

};

#endif
