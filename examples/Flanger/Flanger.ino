// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "LFO.h"
#include "effects/Delay.h"

// BUG: Rate is too fast
#define MIN_RATE (0.001)
#define MAX_RATE (0.01)

#define DELAY_TIME (0.01) // 10ms

#define MIN_TIME (0.001) // DELAY_TIME +/- 2ms
#define MAX_TIME (0.005) // DELAY_TIME +/- 9ms

#define DELAY_SIZE (DEFAULT_SAMPLE_RATE * DELAY_TIME * 4)

Delay effect(DELAY_SIZE);
LFO lfo;

float depth;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Flanger");

  lfo.setWaveform(lfoWaveformSine);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Flanger"));
  zeroStomp.setLabel(0, F("Mix"));
  zeroStomp.setLabel(1, F("Rate"));
  zeroStomp.setLabel(2, F("Depth"));
}

void updateControl(size_t samples) {
  zeroStomp.setMix(zeroStomp.getValue(0) * 0.5); // Don't allow full wet mix
  lfo.setRate(zeroStomp.getValue(1) * (MAX_RATE - MIN_RATE) + MIN_RATE);
  lfo.setScale(zeroStomp.getValue(2) * (MAX_TIME - MIN_TIME) + MIN_TIME);
  effect.setTime(DELAY_TIME + lfo.get());
}

void updateAudio(float *l, float *r) {
  effect.process(l, r);
}
