// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Filter.h"

Filter filter;

#define MIN_Q (0.7071067811865475)
#define MAX_Q (8.0)

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

void updateControl(uint32_t samples) {
  // Frequency
  filter.frequency = mapFloat(min(zeroStomp.getValue(0) + zeroStomp.getExpression(), 4096), 0, 4096, 20, 20000);

  // Resonance
  filter.Q = mapFloat(zeroStomp.getValue(1), 0, 4096, MIN_Q, MAX_Q);

  // Mode
  filter.mode = (FilterMode)map(zeroStomp.getValue(2), 0, 4096, 0, FILTER_MODES);

  // Update the filter state
  filter.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Process samples through filter biquad
  filter.process(l, r);
}
