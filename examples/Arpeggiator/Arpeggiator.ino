// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Pitch.h"
#include "controls/Selector.h"
#include "controls/Knob.h"

#define NUM_PATTERNS (10)
#define NUM_STEPS (8)
const int8_t patterns[NUM_PATTERNS][NUM_STEPS] = {
  { 0,  4,  0,   4,  0,  4,  0,   4 }, // 1 octave up
  { 0, -4,  0,  -4,  0, -4,  0,  -4 }, // 1 octave down
  { 0,  4,  8,   0,  4,  8,  0,   4 }, // 2 octave up
  { 0, -4, -8,   0, -4, -8,  0,  -4 }, // 2 octave down
  { 0,  4,  8,  12,  0,  4,  8,  12 }, // 3 octave up
  { 0, -4, -8, -12,  0, -4, -8, -12 }, // 3 octave down
  { 0,  1,  2,   3,  0,  1,  2,   3 }, // chord up
  { 0, -1, -2,  -3,  0, -1, -2,  -3 }, // chord down
  { 0,  1,  2,   3,  4,  5,  6,   7 }, // chord up, 2 octave
  { 0, -1, -2,  -3, -4, -5, -6,  -7 }, // chord down, 2 octave
};
const char *pattern_labels[NUM_PATTERNS] = {
  "1u", "1d", "2u", "2d", "3u", "3d", "cu", "cd", "2cu", "2cd"
};

#define NUM_SCALES (8)
#define NUM_NOTES (4)
const int8_t scales[NUM_SCALES][NUM_NOTES] = {
  {  0,  4,  7, 11 }, // maj7
  {  0,  4,  7, 10 }, // 7
  {  0,  3,  7, 10 }, // m7
  {  0,  2,  4,  7 }, // 2
  {  0,  2,  3,  7 }, // m2
  {  0,  4,  7,  9 }, // 6
  {  0,  3,  7,  9 }, // m6
  { -1, -1, -1, -1 }, // random
};
const char *scale_labels[NUM_SCALES] = {
  "mj7", "7", "m7", "2", "m2", "6", "m6", "rnd"
};

#define MIN_RATE (DEFAULT_SAMPLE_RATE / 4)
#define MAX_RATE (DEFAULT_SAMPLE_RATE / 64)

Pitch effect;
Selector controlPattern("Pattern", NUM_PATTERNS, pattern_labels), controlScale("Scale", NUM_SCALES, scale_labels);
Knob controlRate("Rate");
size_t pattern_index = 0, timer = 0;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Arpeggiator");

  zeroStomp.setTitle("Arpeggiator");
  zeroStomp.addControls(3, &controlPattern, &controlScale, &controlRate);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  size_t current_rate = mapControl(controlRate.get() + zeroStomp.getExpression(), MIN_RATE, MAX_RATE);

  if (timer > current_rate) {
    timer %= current_rate;
    if (++pattern_index >= NUM_STEPS) pattern_index = 0;

    int8_t scale_index = patterns[controlPattern.get()][pattern_index];
    int8_t octave = scale_index / NUM_NOTES;
    int8_t scale_value = scales[controlScale.get()][abs(scale_index) % NUM_NOTES];
    if (scale_value < 0) scale_value = random(12);
    if (scale_index < 0) scale_value *= -1;

    effect.setShift(scale_value + 12 * octave);
  }

  // Control led
  zeroStomp.setLed(!zeroStomp.isBypassed() ? (MAX_LED * ((pattern_index + 1) % 2)) : 0);
}

void updateAudio(int32_t *l, int32_t *r) {
  timer++;
  effect.process(l, r);
}
