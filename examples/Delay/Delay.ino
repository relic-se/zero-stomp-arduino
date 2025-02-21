// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Delay.h"
#include "LFO.h"
#include "controls/Knob.h"

#define DELAY_SIZE (DEFAULT_SAMPLE_RATE >> 1) // 500ms * channels

#define MIN_TIME (0.1)
#define MAX_TIME (1.0)

#define MIN_RATE (0.01)
#define MAX_RATE (0.1)

#define MAX_WIDTH (0.1)

Delay effect(DELAY_SIZE);
Knob knobMix("Mix"), knobTime("Time"), knobDecay("Decay");

LFO lfo;
Knob knobRate("Rate", CONTROL_MIN), knobWidth("Width", CONTROL_MIN);

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Delay");

  lfo.setWaveform(lfoWaveformSine);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Delay"));
  zeroStomp.addControls(3, &knobMix, &knobTime, &knobDecay);
  zeroStomp.addControls(2, &knobRate, &knobWidth);
}

void updateControl(uint32_t samples) {
  zeroStomp.setMix(knobMix.get(255));
  effect.setDecay(mapControl(knobDecay.get() + zeroStomp.getExpression(), MIN_LEVEL, MAX_LEVEL));

  float current_time = knobTime.getFloat(MIN_TIME, MAX_TIME);
  lfo.setOffset(current_time);
  lfo.setRate(knobRate.getFloat(MIN_RATE, MAX_RATE));
  lfo.setScale(current_time * knobWidth.getFloat(MAX_WIDTH));
  effect.setTime(lfo.get());
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
