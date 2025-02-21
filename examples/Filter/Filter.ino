// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Filter.h"
#include "controls/Knob.h"
#include "controls/Selector.h"

#define MIN_FREQUENCY (20)
#define MAX_FREQUENCY (20000)

#define MIN_Q (0.7071067811865475)
#define MAX_Q (8.0)

const char *mode_labels[FILTER_MODES] = {
  "LP", "HP", "BP", "Ntc"
};

Filter filter;
Knob frequency("Freq"), resonance("Q");
Selector mode("Mode", FILTER_MODES, mode_labels);

// TODO: LFO & controls

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Filter");
  
  zeroStomp.setTitle("Filter");
  zeroStomp.addControls(3, &frequency, &resonance, &mode);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  // Frequency
  filter.frequency = mapControlFloat(frequency.get() + zeroStomp.getExpression(), MIN_FREQUENCY, MAX_FREQUENCY);

  // Resonance
  filter.Q = resonance.getFloat(MIN_Q, MAX_Q);

  // Mode
  filter.mode = (FilterMode)mode.get();

  // Update the filter state
  filter.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Process samples through filter biquad
  filter.process(l, r);
}
