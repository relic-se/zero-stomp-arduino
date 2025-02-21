// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "controls/Knob.h"
#include "controls/Selector.h"

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

#define MIN_RATE (DEFAULT_SAMPLE_RATE / 4)
#define MAX_RATE (DEFAULT_SAMPLE_RATE / 64)

#define FADE_RATE(x) (DEFAULT_SAMPLE_RATE / (MAX_VALUE(int16_t) * x / 1000)) // x=ms
#define MIN_FADE (FADE_RATE(10))
#define MAX_FADE (FADE_RATE(200))

Selector knobPattern("Pattern", NUM_PATTERNS);
Knob knobRate("Rate"), knobProbability("Prob"), fade("Fade");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Stutter");

  randomSeed(analogRead(PIN_ADC_0));

  zeroStomp.setTitle("Stutter");
  zeroStomp.addControls(3, &knobPattern, &knobRate, &knobProbability);
  zeroStomp.addControl(&fade);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

uint8_t current_pattern = 0, pattern_index = 0;
uint8_t current_probability = 0;
bool current_state = true;

size_t timer = 0, current_rate = MIN_RATE;
int16_t fade_timer = 0, fade_rate = MIN_FADE;

void updateControl(uint32_t samples) {
  current_pattern = knobPattern.get();
  current_probability = knobProbability.get(1, UMAX_VALUE(uint8_t) - 1);
  current_rate = mapControl(knobRate.get() + zeroStomp.getExpression(), MIN_RATE, MAX_RATE);
  fade_rate = fade.get(MIN_FADE, MAX_FADE);

  // Control led
  zeroStomp.setLed(!zeroStomp.isBypassed() ? (MAX_LED * current_state) : 0);
}

void updateAudio(int32_t *l, int32_t *r) {
  if (++timer > current_rate) {
    bool last_state = current_state;
    timer = 0;
    
    if (++pattern_index >= NUM_STEPS) pattern_index = 0;
    current_state = (bool)(patterns[current_pattern][pattern_index / (sizeof(uint8_t) * 8)] & (1 << (pattern_index % (sizeof(uint8_t) * 8))));
    
    if (random(UMAX_VALUE(uint8_t)) < current_probability) current_state = false;

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
