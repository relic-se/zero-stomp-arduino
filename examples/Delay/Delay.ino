// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Delay.h"

#define DELAY_SIZE (DEFAULT_SAMPLE_RATE >> 1) // 500ms * channels

#define MIN_TIME (0.1)
#define MAX_TIME (1.0)

Delay effect(DELAY_SIZE);

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Delay");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Delay"));
  zeroStomp.setLabel(0, F("Mix"));
  zeroStomp.setLabel(1, F("Time"));
  zeroStomp.setLabel(2, F("Decay"));
}

void updateControl(uint32_t samples) {
  zeroStomp.setMix(zeroStomp.getValue(0) >> 4);
  effect.setTime(mapFloat(zeroStomp.getValue(1), 0, 4096, MIN_TIME, MAX_TIME));
  effect.setDecay(map(min(zeroStomp.getValue(2) + zeroStomp.getExpression(), 4096), 0, 4096, MIN_LEVEL, MAX_LEVEL));
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
