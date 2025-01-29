// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "ZeroUtils.h" // For MAX_LEVEL

#define MAX_DIST (DEFAULT_SAMPLE_RATE >> 9)

uint16_t dist, counter = 0;
int32_t last_l, last_r;

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

void updateControl(uint32_t samples) {
  dist = map(min(zeroStomp.getValue(0) + zeroStomp.getExpressionValue(), 4096), 0, 4096, 0, MAX_DIST);

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2) >> 4);
}

void updateAudio(int32_t *l, int32_t *r) {
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
