// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _GATE_H
#define _GATE_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"
#include "effects/Envelope.h"

#define GATE_DEFAULT_THRESHOLD (0.05)
#define GATE_DEFAULT_ATTACK (0.01)
#define GATE_DEFAULT_DECAY (0.1)

class Gate : public Effect
{

public:
    Gate(float threshold = GATE_DEFAULT_THRESHOLD, float attack = GATE_DEFAULT_ATTACK, float decay = GATE_DEFAULT_DECAY, size_t sample_rate = DEFAULT_SAMPLE_RATE, uint8_t channels = DEFAULT_CHANNELS);

    void setThreshold(float value);
    void setAttackTime(float value);
    void setDecayTime(float value);

    bool isActive();

    void process(int32_t *l, int32_t *r = nullptr);

private:
    Envelope envelope;
    size_t _sampleRate, _timer;
    size_t _attack_time, _decay_time;
    int16_t _level;

};

#endif
