// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Loopback");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate zeroStomp");
    while (1) { };
  }

  zeroStomp.setTitle("Loopback");
}

void loop() {
  zeroStomp.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Audio processing goes here
}

void updateControl(uint16_t samples) {
  // Control updates go here
}
