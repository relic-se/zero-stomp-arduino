// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Chorus.h"

#define MIN_DEPTH (0.01) // 10ms
#define MAX_DEPTH (0.1) // 100ms

#define MIN_VOICES (2)
#define MAX_VOICES (5)

#define DELAY_SIZE (DEFAULT_SAMPLE_RATE * MAX_DEPTH)

Chorus effect(MAX_DEPTH);

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Chorus");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Chorus"));
  zeroStomp.setLabel(0, F("Mix"));
  zeroStomp.setLabel(1, F("Voices"));
  zeroStomp.setLabel(2, F("Depth"));
}

void updateControl(uint32_t samples) {
  zeroStomp.setMix(zeroStomp.getValue(0) >> 5); // Only allow 0-127
  effect.setVoices(map(zeroStomp.getValue(1), 0, 4096, MIN_VOICES, MAX_VOICES));
  effect.setTime(mapFloat(zeroStomp.getValue(2), 0, 4096, MIN_DEPTH, MAX_DEPTH));
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
