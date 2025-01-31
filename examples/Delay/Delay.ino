// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Delay.h"

#if PICO_RP2350
#define SAMPLE_RATE (DEFAULT_SAMPLE_RATE)
#define CHANNELS (DEFAULT_CHANNELS)
#else
#define SAMPLE_RATE (16000)
#define CHANNELS (2) // BUG: Delay is distorted when mono
#endif

#define MIN_TIME (0.1)
#define MAX_TIME (1.0)

#define DELAY_SIZE (SAMPLE_RATE >> 2) // 250ms * CHANNELS

Delay effect(DELAY_SIZE, MIN_TIME, 0.0, 1.0, SAMPLE_RATE, CHANNELS);

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Delay");

  zeroStomp.setSampleRate(SAMPLE_RATE);
  zeroStomp.setChannels(CHANNELS);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Delay"));
  zeroStomp.setLabel(0, F("Mix"));
  zeroStomp.setLabel(1, F("Time"));
  zeroStomp.setLabel(2, F("Decay"));
}

void updateControl(size_t samples) {
  zeroStomp.setMix(zeroStomp.getValue(0));
  effect.setTime(zeroStomp.getValue(1) * (MAX_TIME - MIN_TIME) + MIN_TIME);
  effect.setDecay(min(zeroStomp.getValue(2) + zeroStomp.getExpressionValue(), 1.0));
}

void updateAudio(float *l, float *r) {
  effect.process(l, r);
}
