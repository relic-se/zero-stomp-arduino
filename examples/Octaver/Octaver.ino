// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "controls/Knob.h"

#define MAX_DIST (DEFAULT_SAMPLE_RATE >> 9)

Knob dist("Dist"), level("Level");
uint16_t current_dist, counter = 0;
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
  zeroStomp.addControls(2, &dist, &level);
}

void updateControl(uint32_t samples) {
  current_dist = mapControl(dist.get() + zeroStomp.getExpression(), MAX_DIST);

  // Update output level through codec
  zeroStomp.setLevel(level.get(255));
}

void updateAudio(int32_t *l, int32_t *r) {
  counter++;
  if (counter >= current_dist) {
    counter = 0;
    last_l = *l;
    last_r = *r;
  } else {
    *l = last_l;
    *r = last_r;
  }
}
