// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Pitch.h"
#include "controls/Knob.h"

#define SEMITONES (24)

Pitch effect;
Knob shift("Shift"), mix("Mix"), level("Level");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Pitch Shifter");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Pitch Shifter"));
  zeroStomp.addControls(3, &shift, &mix, &level);
}

void updateControl(uint32_t samples) {
  effect.setShift(shift.get(-SEMITONES, SEMITONES));

  // Update mix and output level through codec
  zeroStomp.setMix(mix.get(255));
  zeroStomp.setLevel(level.get(255));
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
