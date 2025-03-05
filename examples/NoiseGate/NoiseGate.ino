// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Gate.h"
#include "controls/Knob.h"

#define MIN_THRESHOLD (0.001)
#define MAX_THRESHOLD (0.1)

#define MIN_TIME (0.01)
#define MAX_TIME (0.2)

Gate gate;
Knob threshold("Threshold"), attack("Attack"), decay("Decay");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Noise Gate");

  zeroStomp.setTitle("Noise Gate");
  zeroStomp.addControls(3, &threshold, &attack, &decay);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  gate.setThreshold(threshold.getFloat(MIN_THRESHOLD, MAX_THRESHOLD));
  gate.setAttackTime(attack.getFloat(MIN_TIME, MAX_TIME));
  gate.setDecayTime(decay.getFloat(MIN_TIME, MAX_TIME));
}

void updateAudio(int32_t *l, int32_t *r) {
  gate.process(l, r);
}
