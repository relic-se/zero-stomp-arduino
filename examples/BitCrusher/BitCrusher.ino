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
    Serial.println("Failed to initiate zeroStomp");
    while (1) { };
  }

  zeroStomp.setTitle(F("Bit Crusher"));

  zeroStomp.setLabel(0, F("Mix"));
  zeroStomp.setLabel(1, F("Bits"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(uint32_t samples) {
  // Update wet/dry mix through codec
  zeroStomp.setMix(zeroStomp.getValue(0) >> 4);

  // Calculate bit mask
  bitmask = 0xFFFFFFFF ^ ((1 << (uint32_t)map(min(zeroStomp.getValue(1) + zeroStomp.getExpressionValue(), 4096), 0, 4096, 0, 14)) - 1);

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2) >> 4);
}

void updateAudio(int32_t *l, int32_t *r) {
  // Apply 32-bit mask to audio samples
  *l &= bitmask;
  *r &= bitmask;
}
