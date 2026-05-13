// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "controls/Knob.h"

int16_t bitmask = 0xFFFF;

Knob mix("Mix"), bits("Bits"), level("Level");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Bit Crusher");

  zeroStomp.setTitle("Bit Crusher");
  zeroStomp.addControls(3, &mix, &bits, &level);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(size_t samples) {
  // Update wet/dry mix through codec
  zeroStomp.setMix(mix.get() >> 4);

  // Calculate bit mask
  bitmask = 0xFFFF ^ ((1 << mapControl(bits.get() + zeroStomp.getExpression(), 0, 14)) - 1);
  Serial.println(bitmask, HEX);

  // Update output level through codec
  zeroStomp.setLevel(level.get() >> 4);
}

void updateAudio(int32_t *l, int32_t *r) {
  // Apply 16-bit mask to audio samples
  *l &= bitmask;
  *r &= bitmask;
}
