// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Loopback");

  zeroStomp.setTitle("Loopback");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  // Control updates go here
}

void updateAudio(int32_t *l, int32_t *r) {
  // Audio processing goes here
}
