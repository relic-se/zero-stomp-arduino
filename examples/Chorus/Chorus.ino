// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "LFO.h"
#include "effects/Delay.h"

// BUG: Rate is too fast
#define MIN_RATE (0.01)
#define MAX_RATE (0.1)

#define MIN_DEPTH (0.002) // 4ms +/- 2ms
#define MAX_DEPTH (0.01) // 20ms +/- 10ms

#define DELAY_SIZE (DEFAULT_SAMPLE_RATE * MAX_DEPTH * 2)

Delay effect(DELAY_SIZE);
LFO lfo;

float depth;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Chorus");

  lfo.setWaveform(lfoWaveformSine);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Chorus"));
  zeroStomp.setLabel(0, F("Mix"));
  zeroStomp.setLabel(1, F("Rate"));
  zeroStomp.setLabel(2, F("Depth"));
}

void updateControl(uint32_t samples) {
  zeroStomp.setMix(zeroStomp.getValue(0) >> 5); // Only allow 0-127
  lfo.setRate(mapFloat(zeroStomp.getValue(1), 0, 4096, MIN_RATE, MAX_RATE));

  depth = mapFloat(zeroStomp.getValue(2), 0, 4096, MIN_DEPTH, MAX_DEPTH);
  lfo.setOffset(depth * 2);
  lfo.setScale(depth);

  effect.setTime(lfo.get());
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
