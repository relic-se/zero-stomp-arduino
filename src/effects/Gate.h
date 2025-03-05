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
    int16_t getLevel();

    void applyScale(bool value);
    void process(int32_t *l, int32_t *r = nullptr);

private:
    Envelope envelope;
    size_t _sampleRate;
    int16_t _attack_rate, _decay_rate;
    int32_t _level = 0;
    bool _scale = true;

};

#endif
