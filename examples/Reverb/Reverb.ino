// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Reverb.h"

Reverb effect;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Reverb");

  #if !PICO_RP2350
  zeroStomp.setSampleRate(8000);
  zeroStomp.setChannels(1);
  effect.setSampleRate(8000);
  effect.setChannels(1);
  #endif

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Reverb"));

  zeroStomp.setLabel(0, F("Size"));
  zeroStomp.setLabel(1, F("Damping"));
  zeroStomp.setLabel(2, F("Mix"));
}

void updateControl(uint32_t samples) {
    effect.setRoomSize(mapFloat(zeroStomp.getValue(0), 0, 4096, 0.0, 1.0));
    effect.setDamping(mapFloat(zeroStomp.getValue(1), 0, 4096, 0.0, 1.0));
    zeroStomp.setMix(zeroStomp.getValue(2) >> 4);
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
