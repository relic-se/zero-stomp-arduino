// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

ZeroStomp device;

int32_t bitmask = 0xFFFFFFFF;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Bit Crusher");

  if (!device.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  device.setTitle(F("Bit Crusher"));

  device.setLabel(0, F("Mix"));
  device.setLabel(1, F("Bits"));
  device.setLabel(2, F("Level"));
}

void loop() {
  device.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Apply 32-bit mask to audio samples
  *l &= bitmask;
  *r &= bitmask;
}

void updateControl(uint16_t samples) {
  // Update wet/dry mix through codec
  device.setMix(device.getValue(0) >> 4);

  // Calculate bit mask
  bitmask = 0xFFFFFFFF ^ ((1 << (uint32_t)map(device.getValue(1), 0, 4096, 0, 14)) - 1);

  // Update output level through codec
  device.setLevel(device.getValue(2) >> 4);
}
