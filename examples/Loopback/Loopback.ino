// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

ZeroStomp device;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Loopback");

  if (!device.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  device.setTitle("Loopback");
}

void loop() {
  device.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Audio processing goes here
}

void updateControl(uint16_t samples) {
  // Control updates go here
}
