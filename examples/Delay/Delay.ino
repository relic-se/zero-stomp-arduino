// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "ZeroAudio.h" // Needed for mix_down_sample

ZeroStomp device;

#define DELAY_SIZE (DEFAULT_SAMPLE_RATE >> 1) // 500ms * channels
#define DELAY_SUBBITS 8

#define MIN_RATE (1 << (DELAY_SUBBITS - 1)) // 1000ms
#define MAX_RATE (1 << (DELAY_SUBBITS + 4)) // 31.25ms

int16_t buffer[2][DELAY_SIZE];
size_t buffer_pos;
uint32_t buffer_rate = 1 << DELAY_SUBBITS;
uint16_t decay = 1 << 15; // 0 - 65536, defaults to half

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Loopback");

  // Fill buffers with silence
  memset((void *)buffer[0], 0, DELAY_SIZE * sizeof(int16_t));
  memset((void *)buffer[1], 0, DELAY_SIZE * sizeof(int16_t));
  buffer_pos = 0;

  if (!device.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void loop() {
  device.update();
}

int32_t updateDelay(int32_t sample, int16_t *b) {
  // Get current echo value from buffer
  int32_t echo = (int32_t)b[buffer_pos >> DELAY_SUBBITS];

  // Apply decay to echo and add current sample
  int32_t mix = ((echo * decay) >> 16) + sample;

  // Apply dynamic range compression
  mix = mix_down_sample(mix, MIX_DOWN_SCALE(2));

  // Update echo buffer
  for (size_t i = buffer_pos >> DELAY_SUBBITS; i < (buffer_pos + buffer_rate) >> DELAY_SUBBITS; i++) {
      b[i % DELAY_SIZE] = (int16_t)mix;
  }

  // Return original echo value
  return echo;
}

void updateAudio(int32_t *l, int32_t *r) {
  *l = updateDelay(*l, (int16_t *)&buffer[0]);
  *r = updateDelay(*r, (int16_t *)&buffer[1]);
  buffer_pos = (buffer_pos + buffer_rate) % (DELAY_SIZE << DELAY_SUBBITS);
}

void updateControl() {
  device.setMix(device.getAdcValue(0) >> 4);
  // TODO: Logarithmic
  buffer_rate = map(device.getAdcValue(1), 0, 4096, MAX_RATE, MIN_RATE);
  decay = map(device.getAdcValue(2), 0, 4096, 0, 65535);
}
