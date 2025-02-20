// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "controls/Knob.h"

#define MIN_THRESHOLD (MAX_LEVEL >> 4)
#define MAX_THRESHOLD (MAX_LEVEL >> 12)

Knob gain("Fuzz"), level("Level");
int32_t threshold;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Fuzz");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Fuzz"));
  zeroStomp.addControls(2, &gain, &level);
}

void updateControl(uint32_t samples) {
  threshold = mapControl(gain.get() + zeroStomp.getExpression(), MIN_THRESHOLD, MAX_THRESHOLD);

  // Update output level through codec
  zeroStomp.setLevel(level.get(255));
}

int32_t fuzz(int32_t sample) {
  return map(sample, -threshold, threshold, -MAX_LEVEL, MAX_LEVEL);
}

void updateAudio(int32_t *l, int32_t *r) {
  *l = fuzz(*l);
  *r = fuzz(*r);
}
