// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ENVELOPE_H
#define _ENVELOPE_H

#include "Arduino.h"
#include "ZeroStomp.h"
#include "effects/Effect.h"

#define ENVELOPE_DEFAULT_RISE (0.01)
#define ENVELOPE_DEFAULT_FALL (0.0001)

#define ENVELOPE_DEFAULT_ATTACK (0.01)
#define ENVELOPE_DEFAULT_RELEASE (0.0001)

typedef void (*EnvelopeAttackCallback)(void);
typedef void (*EnvelopeReleaseCallback)(void);

class Envelope : public Effect
{

public:
    Envelope(float attack = ENVELOPE_DEFAULT_ATTACK, float release = ENVELOPE_DEFAULT_RELEASE, float rise = ENVELOPE_DEFAULT_RISE, float fall = ENVELOPE_DEFAULT_FALL, uint8_t channels = DEFAULT_CHANNELS);

    void setAttackLevel(float value);
    void setAttackCallback(EnvelopeAttackCallback cb);
    
    void setReleaseLevel(float value);
    void setReleaseCallback(EnvelopeReleaseCallback cb);

    bool isActive();
    bool didAttack();
    bool didRelease();

    void setRise(float value);
    void setFall(float value);

    void process(int32_t *l, int32_t *r = nullptr);
    float get();
    int32_t get_scaled();

protected:
    void reset();

private:
    int16_t _attack, _release;
    EnvelopeAttackCallback _attack_cb = nullptr;
    EnvelopeReleaseCallback _release_cb = nullptr;
    bool _active = false, _did_attack = false, _did_release = false;

    int16_t _rise, _fall;
    int32_t _accum;

};

#endif
