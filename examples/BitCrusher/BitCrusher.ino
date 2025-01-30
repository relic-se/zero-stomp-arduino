// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

int32_t bitmask = 0xFFFFFFFF;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Bit Crusher");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Bit Crusher"));

  zeroStomp.setLabel(0, F("Mix"));
  zeroStomp.setLabel(1, F("Bits"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(size_t samples) {
  // Update wet/dry mix through codec
  zeroStomp.setMix(zeroStomp.getValue(0));

  // Calculate bit mask
  bitmask = 0xFFFFFFFF ^ ((1 << (uint32_t)round(min(zeroStomp.getValue(1) + zeroStomp.getExpressionValue(), 1.0) * 14.0)) - 1);

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2));
}

void updateAudio(float *l, float *r) {
  // Apply 32-bit mask to audio samples
  *l &= bitmask;
  *r &= bitmask;
}
