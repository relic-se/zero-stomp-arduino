// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

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

  zeroStomp.setLabel(0, F("Fuzz"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(uint32_t samples) {
  threshold = map(min(zeroStomp.getValue(0) + zeroStomp.getExpressionValue(), 4096), 0, 4096, MAX_LEVEL >> 4, MAX_LEVEL >> 12);

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2) >> 4);
}

int32_t fuzz(int32_t sample) {
  return map(sample, -threshold, threshold, -MAX_LEVEL, MAX_LEVEL);
}

void updateAudio(int32_t *l, int32_t *r) {
  *l = fuzz(*l);
  *r = fuzz(*r);
}
