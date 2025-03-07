// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include <AMY-Arduino.h>

#include "ZeroStomp.h"
#include "effects/Detect.h"
#include "effects/Envelope.h"

#define LEVEL (0.1)

AMY amy;

Detect detect(DEFAULT_FFT_SIZE, AMY_SAMPLE_RATE);
Envelope envelope;

int16_t * buffer = nullptr;
size_t buffer_index = AMY_BLOCK_SIZE;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Synth");

  zeroStomp.setTitle("Synth");

  // Match settings with AMY
  zeroStomp.setSampleRate(AMY_SAMPLE_RATE);
  zeroStomp.setBitsPerSample(AMY_BYTES_PER_SAMPLE * 8);
  zeroStomp.setChannels(AMY_NCHANS);
  zeroStomp.setBufferSize(AMY_BLOCK_SIZE / AMY_NCHANS);
  // TODO: Set num buffers?

  // Start up AMY
  amy.begin(1, 0, 0, 0);

  // Load Patch
  struct event e = amy.default_event();
  e.osc = 0;
  e.wave = SINE;
  amy.add_event(e);

  // Initialize codec, etc
  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

float freq, level;

void updateControl(uint32_t samples) {
  if (envelope.isActive() && detect.ready() && !level) {
    // Get note properties
    freq = detect.getFrequency();
    level = min(envelope.get() / LEVEL, 1.0);

    // Press note
    struct event e = amy.default_event();
    e.freq_coefs[COEF_CONST] = freq;
    e.velocity = level;
    amy.add_event(e);
  } else if (!envelope.isActive()) {
    // Reset note properties
    freq = level = 0.0;

    // Release note
    struct event e = amy.default_event();
    e.velocity = 0.0;
    amy.add_event(e);
  }

  // Control led
  if (!zeroStomp.isBypassed()) {
    zeroStomp.setLed(level * MAX_LED);
  } else {
    zeroStomp.setLed(0);
  }
}

void updateAudio(int32_t *l, int32_t *r) {
  // Check to see if envelope is active
  envelope.process(l, r);

  // Reset detection if envelope triggered
  if (envelope.didAttack()) detect.flush();

  // Add sample data to detector if active
  if (envelope.isActive()) detect.process(l, r);

  if (buffer_index < AMY_BLOCK_SIZE) {
    // Transfer AMY buffer
    *l = buffer[buffer_index++];
    *r = buffer[buffer_index++];
  } else {
    // Update AMY buffer
    buffer = (int16_t *)amy.render_to_buffer();
    buffer_index = 0;
  }
}
