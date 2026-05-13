// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Pitch.h"
#include "controls/Knob.h"

#define SEMITONES (24)

Pitch effect;
Knob shift("Shift"), mix("Mix"), level("Level");

// TODO: Use selector for semitones

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Pitch Shifter");

  zeroStomp.setTitle("Pitch Shifter");
  zeroStomp.addControls(3, &shift, &mix, &level);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(size_t samples) {
  effect.setShift(shift.get(-SEMITONES, SEMITONES));

  // Update mix and output level through codec
  zeroStomp.setMix(mix.get(255));
  zeroStomp.setLevel(level.get(255));
}

void updateAudio(int16_t *l, int16_t *r) {
  effect.process(l, r);
}
