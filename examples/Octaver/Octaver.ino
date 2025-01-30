// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

#define MAX_DIST (DEFAULT_SAMPLE_RATE >> 9)

size_t dist, counter = 0;
float last_l, last_r;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Octaver");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Octaver"));

  zeroStomp.setLabel(0, F("Dist"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(size_t samples) {
  dist = min(zeroStomp.getValue(0) + zeroStomp.getExpressionValue(), 1.0) * MAX_DIST;

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2));
}

void updateAudio(float *l, float *r) {
  counter++;
  if (counter >= dist) {
    counter = 0;
    last_l = *l;
    last_r = *r;
  } else {
    *l = last_l;
    *r = last_r;
  }
}
