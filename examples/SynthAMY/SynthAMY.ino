// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include <AMY-Arduino.h>
#include "effects/Detect.h"
#include "effects/Envelope.h"

#define LEVEL (0.1)

AMY amy;

Detect detect(DEFAULT_FFT_SIZE, AMY_SAMPLE_RATE);
Envelope envelope;

int16_t * buffer = nullptr;
size_t buffer_index = AMY_BLOCK_SIZE;

struct event note;

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

  note = amy.default_event();
  note.osc = 0;
  note.wave = SINE;

  // Initialize codec, etc
  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  if (envelope.isActive() && detect.ready() && !note.velocity) {
    // Get note properties
    float freq = detect.getFrequency();
    float level = min(envelope.get() / LEVEL, 1.0);

    // Press note
    note.freq_coefs[COEF_CONST] = freq;
    note.velocity = level;
    amy.add_event(note);
  } else if (!envelope.isActive()) {
    // Release note
    note.velocity = 0;
    amy.add_event(note);
  }

  // Control led
  if (!zeroStomp.isBypassed()) {
    zeroStomp.setLed(envelope.isActive() ? MAX_LED : (MAX_LED >> 1));
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
