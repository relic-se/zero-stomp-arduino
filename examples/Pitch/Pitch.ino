// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Pitch.h"

#define SEMITONES (24)

Pitch effect;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Pitch Shifter");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Pitch Shifter"));

  zeroStomp.setLabel(0, F("Shift"));
  zeroStomp.setLabel(1, F("Mix"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(uint32_t samples) {
  effect.setShift(map(zeroStomp.getValue(0), 0, 4096, -SEMITONES, SEMITONES));

  // Update mix and output level through codec
  zeroStomp.setMix(zeroStomp.getValue(1) >> 4);
  zeroStomp.setLevel(zeroStomp.getValue(2) >> 4);
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
