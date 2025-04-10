// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/MultiTapDelay.h"
#include "controls/Knob.h"
#include "controls/Selector.h"

#define MIN_TIME (0.1)
#define MAX_TIME (1.0)
#define TIME_EPSILON (0.01)

// Eighth note positions
const Tap taps[8] = {
    { 0.125, MAX_LEVEL },
    { 0.25,  1.0 },
    { 0.375, 1.0 },
    { 0.5,   1.0 },
    { 0.625, 1.0 },
    { 0.75,  1.0 },
    { 0.875, 1.0 },
    DefaultTap,
};

typedef struct tap_pattern_t {
    size_t count;
    Tap **pattern;
} TapPattern;

#define NUM_PATTERNS (8)
const TapPattern patterns[NUM_PATTERNS] = {
    { 1, { &taps[7] } },                                // whole
    { 2, { &taps[3], &taps[7] } },                      // half
    { 3, { &taps[2], &taps[5], &taps[7] } },            // dotted quarter
    { 4, { &taps[1], &taps[3], &taps[5], &taps[7] } },  // quarter
    { 8, &taps },                                       // eighth
    { 4, { &taps[0], &taps[1], &taps[4], &taps[5] } },
    { 3, { &taps[1], &taps[6], &taps[7] } },
    { 5, { &taps[1], &taps[2], &taps[4], &taps[5], &taps[7] } },
};

MultiTapDelay effect();
Knob knobMix("Mix"), knobDecay("Decay"), knobTime("Time");
Selector knobPattern("Pattern", NUM_PATTERNS);

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Multi Tap Delay");

  zeroStomp.setTitle("Multi Tap Delay");
  zeroStomp.addControls(3, &knobMix, &knobTime, &knobPattern);
  zeroStomp.addControls(1, &knobDecay);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

size_t pattern = -1;

void updateControl(uint32_t samples) {
  zeroStomp.setMix(knobMix.get(255));
  effect.setDecay(mapControl(knobDecay.get() + zeroStomp.getExpression(), MIN_LEVEL, MAX_LEVEL));

  // TODO: Use tempo instead of time
  float time = knobTime.getFloat(MIN_TIME, MAX_TIME);
  if (abs(time - effect.getTime()) > TIME_EPSILON) {
    effect.setTime(time);
  }

  size_t current_pattern = knobPattern.get();
  if (pattern != current_pattern) {
    pattern = current_pattern;
    effect.setTaps(patterns[pattern].count, patterns[pattern].pattern)
  }
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
