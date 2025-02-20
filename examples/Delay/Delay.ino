// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Delay.h"
#include "controls/Knob.h"

#define DELAY_SIZE (DEFAULT_SAMPLE_RATE >> 1) // 500ms * channels

#define MIN_TIME (0.1)
#define MAX_TIME (1.0)

Delay effect(DELAY_SIZE);
Knob knobMix("Mix"), knobTime("Time"), knobDecay("Decay");

// TODO: LFO & controls

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Delay");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Delay"));
  zeroStomp.addControls(3, &knobMix, &knobTime, &knobDecay);
}

void updateControl(uint32_t samples) {
  zeroStomp.setMix(knobMix.get(255));
  effect.setTime(knobTime.getFloat(MIN_TIME, MAX_TIME));
  effect.setDecay(mapControl(knobDecay.get() + zeroStomp.getExpression(), MIN_LEVEL, MAX_LEVEL));
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
