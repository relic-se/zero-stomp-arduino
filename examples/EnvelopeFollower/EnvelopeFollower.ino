// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Envelope.h"
#include "effects/Filter.h"

#define MIN_RISE (0.25)
#define MAX_RISE (0.9)

#define MIN_FREQUENCY (40) // hz
#define MIN_RANGE (100) // hz
#define MAX_RANGE (10000) // hz

#define MIN_Q (0.7071067811865475)
#define MAX_Q (8.0)

Envelope envelope;
Filter filter(FilterMode::BAND_PASS, MIN_FREQUENCY, MIN_Q);

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Envelope Follower");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
  
  zeroStomp.setTitle(F("Envelope Follower"));

  zeroStomp.setLabel(0, F("Sensitivity"));
  zeroStomp.setLabel(1, F("Range"));
  zeroStomp.setLabel(2, F("Q"));
}

float range = MIN_RANGE;

void updateControl(uint32_t samples) {
  // Envelope rise rate
  envelope.setRise(mapFloat(zeroStomp.getValue(0), 0, 4096, MIN_RISE, MAX_RISE));

  // Filter range
  range = mapFloat(min(zeroStomp.getValue(1) + zeroStomp.getExpressionValue(), 4096), 0, 4096, MIN_RANGE, MAX_RANGE);

  // Resonance
  filter.Q = mapFloat(zeroStomp.getValue(2), 0, 4096, MIN_Q, MAX_Q);

  // Set the filter frequency scaled by the envelope
  filter.frequency = MIN_FREQUENCY + range * envelope.get();

  // Update the filter state
  filter.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Update the state of the envelope (audio is not altered)
  envelope.process(l, r);

  // Process samples through band pass filter biquad
  filter.process(l, r);
}
