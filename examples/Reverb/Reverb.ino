// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Reverb.h"
#include "controls/Knob.h"

Reverb effect;
Knob size("Size"), damping("Damping"), mix("Mix");

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

  zeroStomp.setTitle("Reverb");
  zeroStomp.addControls(3, &size, &damping, &mix);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
    effect.setRoomSize(size.getFloat());
    effect.setDamping(damping.getFloat());
    zeroStomp.setMix(mix.get(255));
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
