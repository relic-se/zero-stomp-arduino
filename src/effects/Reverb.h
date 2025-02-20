// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _REVERB_H
#define _REVERB_H

#include "Arduino.h"
#include "ZeroStomp.h" // For DEFAULT_CHANNELS
#include "effects/Effect.h"
#include "freeverb/revmodel.hpp"

typedef enum {
    NORMAL = 0,
    FREEZE = 1
} ReverbMode;

class Reverb : public Effect
{

public:
    Reverb(ReverbMode mode = NORMAL, float room_size = initialroom, float damping = initialdamp, float width = initialwidth, int16_t mix = MAX_VALUE(int16_t), size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    void setMode(ReverbMode value);
    void setRoomSize(float value);
    void setDamping(float value);
    void setWidth(float value);
    void setMix(int16_t value) override;
    void setSampleRate(size_t value);
    void setChannels(uint8_t value) override;

    void process(int32_t *l, int32_t *r = nullptr);

private:
    revmodel _model;

};

#endif
