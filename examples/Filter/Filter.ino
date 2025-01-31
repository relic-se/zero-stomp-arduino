// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Filter.h"

#define MIN_Q (0.7071067811865475)
#define MAX_Q (8.0)

#define MIN_FREQUENCY 20
#define MAX_FREQUENCY 20000

Filter filter;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Filter");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
  
  zeroStomp.setTitle(F("Filter"));

  zeroStomp.setLabel(0, F("Freq"));
  zeroStomp.setLabel(1, F("Q"));
  zeroStomp.setLabel(2, F("Mode"));
}

void updateControl(size_t samples) {
  // Frequency
  filter.frequency = pow(min(zeroStomp.getValue(0) + zeroStomp.getExpressionValue(), 1.0), 3.0) * (MAX_FREQUENCY - MIN_FREQUENCY) + MIN_FREQUENCY;

  // Resonance
  filter.Q = zeroStomp.getValue(1) * (MAX_Q - MIN_Q) + MIN_Q;

  // Mode
  filter.mode = (FilterMode)min(floor(zeroStomp.getValue(2) * FILTER_MODES), FILTER_MODES - 1);

  // Update the filter state
  filter.update();
}

void updateAudio(float *l, float *r) {
  // Process samples through filter biquad
  filter.process(l, r);
}
