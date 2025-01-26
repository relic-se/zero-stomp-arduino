// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "ZeroFilter.h"

ZeroStomp device;
ZeroFilter filter;

#define MIN_Q (0.7071067811865475)
#define MAX_Q (2.0)

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Filter");

  if (!device.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  device.setTitle(F("Filter"));

  device.setLabel(0, F("Freq"));
  device.setLabel(1, F("Q"));
  device.setLabel(2, F("Mode"));
}

void loop() {
  device.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Process samples through filter biquad
  *l = filter.process(*l);
  *r = filter.process(*r);
}

void updateControl(uint16_t samples) {
  // Frequency
  filter.frequency = (float)map(device.getValue(0), 0, 4096, 20, 20000);

  // Resonance
  filter.Q = (float)map(device.getValue(1), 0, 4096, MIN_Q, MAX_Q);

  // Mode
  filter.mode = (FilterMode)map(device.getValue(2), 0, 4096, 0, FILTER_MODES);

  // Update the filter state
  filter.update();
}
