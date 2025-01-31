// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

#define MIN_GAIN 0.0
#define MAX_GAIN 40.0

float gain, clip_level;

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

void updateControl(size_t samples) {
  gain = dbToLinear(min(zeroStomp.getValue(0) + zeroStomp.getExpressionValue(), 1.0) * (MAX_GAIN - MIN_GAIN) + MIN_GAIN);
  clip_level = 1.0 - zeroStomp.getValue(1);

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2));
}

void updateAudio(float *l, float *r) {
  // Apply gain to signal
  *l *= gain;
  *r *= gain;

  // Hard clip
  *l = clip(*l, clip_level);
  *r = clip(*r, clip_level);
}
