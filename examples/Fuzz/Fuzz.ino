// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

#define MIN_THRESHOLD (0.0625) // ldexp(1.0, -4)
#define MAX_THRESHOLD (0.000244140625) // ldexp(1.0, -12)

float threshold;

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

void updateControl(size_t samples) {
  threshold = min(zeroStomp.getValue(0) + zeroStomp.getExpressionValue(), 1.0) * (MAX_THRESHOLD - MIN_THRESHOLD) + MIN_THRESHOLD;

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2));
}

float fuzz(float sample) {
  return ((x + threshold) / threshold) - 1.0;
}

void updateAudio(float *l, float *r) {
  *l = fuzz(*l);
  *r = fuzz(*r);
}
