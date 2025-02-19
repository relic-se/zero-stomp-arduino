// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

int16_t gain;
int32_t clip_level;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Distortion");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Distortion"));

  zeroStomp.setLabel(0, F("Gain"));
  zeroStomp.setLabel(1, F("Clip"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(uint32_t samples) {
  gain = min(zeroStomp.getValue(0) + zeroStomp.getExpression(), 4096);
  clip_level = ((4096 - zeroStomp.getValue(1)) << 2) - 1;

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2) >> 4);
}

void updateAudio(int32_t *l, int32_t *r) {
  // Apply gain to signal
  *l = scale(*l, gain, 6);
  *r = scale(*r, gain, 6);

  // TODO: Soft clip?

  // Hard clip
  *l = clip(*l, clip_level);
  *r = clip(*r, clip_level);
}
