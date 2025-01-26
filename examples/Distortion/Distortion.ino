// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

ZeroStomp device;

int32_t gain, clip;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Distortion");

  if (!device.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  device.setMix(255);

  device.setTitle(F("Distortion"));

  device.setLabel(0, F("Gain"));
  device.setLabel(1, F("Clip"));
  device.setLabel(2, F("Level"));
}

void loop() {
  device.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Apply gain to signal
  *l = (*l * gain) >> 6;
  *r = (*r * gain) >> 6;

  // TODO: Soft clip?

  // Hard clip
  *l = min(max(*l, -clip), clip);
  *r = min(max(*r, -clip), clip);
}

void updateControl() {
  gain = device.getValue(0);
  clip = ((4096 - device.getValue(1)) << 2) - 1;

  // Update output level through codec
  device.setLevel(device.getValue(2) >> 4);
}
