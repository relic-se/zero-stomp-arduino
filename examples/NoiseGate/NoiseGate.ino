// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Gate.h"
#include "controls/Knob.h"

#define MIN_THRESHOLD (0.0)
#define MAX_THRESHOLD (0.02)

#define MIN_ATTACK_TIME (0.002)
#define MAX_ATTACK_TIME (0.1)

#define MIN_DECAY_TIME (0.05)
#define MAX_DECAY_TIME (0.5)

#define DIGITAL (0)

Gate gate;
Knob threshold("Threshold"), attack("Attack"), decay("Decay");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Noise Gate");

  zeroStomp.setTitle("Noise Gate");
  zeroStomp.addControls(3, &threshold, &attack, &decay);

  #if !DIGITAL
  zeroStomp.setMix(0);
  gate.applyScale(false);
  #endif

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  // Update noise gate settings
  gate.setThreshold(mapFloat(pow(threshold.getFloat(), 3), 0.0, 1.0, MIN_THRESHOLD, MAX_THRESHOLD));
  gate.setAttackTime(attack.getFloat(MIN_ATTACK_TIME, MAX_ATTACK_TIME));
  gate.setDecayTime(decay.getFloat(MIN_DECAY_TIME, MAX_DECAY_TIME));

  // Control led
  if (!zeroStomp.isBypassed()) {
    zeroStomp.setLed((MAX_LED >> 1) + scale(MAX_LEVEL >> 1, gate.getLevel()));
  } else {
    zeroStomp.setLed(0);
  }

  #if !DIGITAL
  zeroStomp.setLevel(gate.getLevel() >> (BITS_PER_SAMPLE - 9));
  #endif
}

void updateAudio(int32_t *l, int32_t *r) {
  gate.process(l, r);
}
