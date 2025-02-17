// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

#define NUM_STEPS 16
#define NUM_PATTERNS 4

const uint8_t patterns[NUM_PATTERNS][NUM_STEPS / (sizeof(uint8_t) * 8)] = {
    { 0b11001100, 0b11001100 },
    { 0b10101010, 0b10101010 },
    { 0b10100010, 0b10100000 },
    { 0b10010101, 0b10010010 },
};

#define MIN_RATE (DEFAULT_SAMPLE_RATE / 4)
#define MAX_RATE (DEFAULT_SAMPLE_RATE / 16)

#define MIN_SLEW 16
#define MAX_SLEW 256

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Stutter");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Stutter"));

  zeroStomp.setLabel(0, F("Pattern"));
  zeroStomp.setLabel(1, F("Rate"));
  zeroStomp.setLabel(2, F("Slew"));
}

size_t pattern_timer = 0, slew_timer = 0;
uint8_t pattern_index = 0;
bool current_step = true;

uint8_t pattern = 0;
size_t rate = MIN_RATE;
uint8_t slew = MIN_SLEW, current_slew = MIN_SLEW;

void updateControl(uint32_t samples) {
  pattern = map(zeroStomp.getValue(0), 0, 4096, 0, NUM_PATTERNS);

  rate = map(min(zeroStomp.getValue(1) + zeroStomp.getExpressionValue(), 4096), 0, 4096, MIN_RATE, MAX_RATE);

  slew = map(zeroStomp.getValue(2), 0, 4096, MIN_SLEW, MAX_SLEW);

  // Control led
  zeroStomp.setLed(!zeroStomp.isBypassed() ? (MAX_LED * current_step) : 0);
}

void updateAudio(int32_t *l, int32_t *r) {
  if (++pattern_timer > rate) {
    pattern_timer = 0;
    if (++pattern_index >= NUM_STEPS) pattern_index = 0;

    slew_timer = slew;
    current_slew = slew; // NOTE: Copy of slew prevents unintended behavior if changed within updateControl

    current_step = (bool)(patterns[pattern][pattern_index / (sizeof(uint8_t) * 8)] & (1 << (pattern_index % (sizeof(uint8_t) * 8))));
  }

  if (slew_timer > 0) {
    size_t level = current_step ? (current_slew - slew_timer) : slew_timer;
    *l = (*l * level) / current_slew;
    *r = (*r * level) / current_slew;
    slew_timer--;
  }
}
