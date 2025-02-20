// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "controls/Knob.h"

Knob knobGain("Gain"), knobClip("Clip"), knobLevel("Level");

int16_t current_gain;
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
  zeroStomp.addControls(3, &knobGain, &knobClip, &knobLevel);
}

void updateControl(uint32_t samples) {
  current_gain = min(knobGain.get() + zeroStomp.getExpression(), CONTROL_MAX);
  clip_level = knobClip.get(MAX_LEVEL / 2, 0);

  // Update output level through codec
  zeroStomp.setLevel(knobLevel.get(255));
}

void updateAudio(int32_t *l, int32_t *r) {
  // Apply gain to signal
  *l = scale(*l, current_gain, 6);
  *r = scale(*r, current_gain, 6);

  // TODO: Soft clip?

  // Hard clip
  *l = clip(*l, clip_level);
  *r = clip(*r, clip_level);
}
