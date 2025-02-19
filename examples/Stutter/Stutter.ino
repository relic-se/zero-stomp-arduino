// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"

//#define RANDOM // NOTE: Uncomment this line to control the stutter using a random number generator

#ifndef RANDOM
#define NUM_STEPS 16
#define NUM_PATTERNS 8
const uint8_t patterns[NUM_PATTERNS][NUM_STEPS / (sizeof(uint8_t) * 8)] = {
  { 0b11001100, 0b11001100 },
  { 0b10101010, 0b10101010 },
  { 0b10100010, 0b10100000 },
  { 0b10010101, 0b10010010 },
  { 0b11101110, 0b11101110 },
  { 0b10000010, 0b10000010 },
  { 0b10000100, 0b10000100 },
  { 0b11111110, 0b11101110 },
};
#endif

#define MIN_RATE (DEFAULT_SAMPLE_RATE / 4)
#define MAX_RATE (DEFAULT_SAMPLE_RATE / 64)

#define FADE_RATE(x) (DEFAULT_SAMPLE_RATE / (MAX_VALUE(int16_t) * x / 1000)) // x=ms
#define MIN_FADE (FADE_RATE(10))
#define MAX_FADE (FADE_RATE(200))

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Stutter");

  #ifdef RANDOM
  randomSeed(analogRead(PIN_ADC_0));
  #endif

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Stutter"));

  #ifndef RANDOM
  zeroStomp.setLabel(0, F("Pattern"));
  #else
  zeroStomp.setLabel(0, F("Prob"));
  #endif
  zeroStomp.setLabel(1, F("Rate"));
  zeroStomp.setLabel(2, F("Fade"));
}

#ifndef RANDOM
uint8_t pattern = 0;
uint8_t pattern_index = 0;
#else
uint8_t probability = 0;
#endif

bool current_state = true;

size_t timer = 0;
size_t rate = MIN_RATE;

int16_t fade_timer = 0;
int16_t fade_rate = MIN_FADE;

void updateControl(uint32_t samples) {
  #ifndef RANDOM
  pattern = map(zeroStomp.getValue(0), 0, 4096, 0, NUM_PATTERNS);
  #else
  probability = map(zeroStomp.getValue(0), 0, 4096, 1, UMAX_VALUE(uint8_t) - 1);
  #endif
  rate = map(min(zeroStomp.getValue(1) + zeroStomp.getExpression(), 4096), 0, 4096, MIN_RATE, MAX_RATE);
  fade_rate = map(zeroStomp.getValue(2), 0, 4096, MIN_FADE, MAX_FADE);

  // Control led
  zeroStomp.setLed(!zeroStomp.isBypassed() ? (MAX_LED * current_state) : 0);
}

void updateAudio(int32_t *l, int32_t *r) {
  if (++timer > rate) {
    bool last_state = current_state;
    timer = 0;
    
    #ifndef RANDOM
    if (++pattern_index >= NUM_STEPS) pattern_index = 0;
    current_state = (bool)(patterns[pattern][pattern_index / (sizeof(uint8_t) * 8)] & (1 << (pattern_index % (sizeof(uint8_t) * 8))));
    #else
    current_state = random(UMAX_VALUE(uint8_t)) > probability;
    #endif

    if (last_state != current_state) fade_timer = MAX_VALUE(int16_t);
  }

  if (fade_timer > 0) {
    int16_t level = current_state ? (MAX_VALUE(int16_t) - fade_timer) : fade_timer;
    *l = scale<int16_t>(*l, level);
    *r = scale<int16_t>(*r, level);
    fade_timer -= fade_rate;
  } else if (!current_state) {
    *l = 0;
    *r = 0;
  }
}
