// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Detect.h"

Detect detect;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Tuner");

  zeroStomp.setTitle("Chromatic Tuner");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  Serial.println(detect.getFrequency());
}

void updateAudio(int32_t *l, int32_t *r) {
  detect.process(l, r);
}
