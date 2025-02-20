// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Envelope.h"
#include "effects/Filter.h"
#include "controls/Knob.h"

#define MIN_RISE (0.001)
#define MAX_RISE (0.01)

#define MIN_FREQUENCY (200) // hz
#define MIN_RANGE (200) // hz
#define MAX_RANGE (10000) // hz

#define MIN_Q (0.7071067811865475)
#define MAX_Q (8.0)

#define GAIN (5.0)
#define GAIN_LED (3.0)
#define MIN_LED (0.5)

Envelope envelope;
Filter filter(FilterMode::LOW_PASS, MIN_FREQUENCY, MIN_Q);
Knob sensitivity("Sensitivity"), range("Range"), resonance("Q");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Envelope Follower");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
  
  zeroStomp.setTitle(F("Envelope Follower"));
  zeroStomp.addControls(3, &sensitivity, &range, &resonance);
}

void updateControl(uint32_t samples) {
  // Envelope rise rate
  envelope.setRise(sensitivity.getFloat(MIN_RISE, MAX_RISE));

  // Filter range
  float current_range = mapControlFloat(range.get() + zeroStomp.getExpression(), MIN_RANGE, MAX_RANGE);

  // Resonance
  filter.Q = resonance.getFloat(MIN_Q, MAX_Q);

  // Set the filter frequency scaled by the envelope
  float level = min(envelope.get() * GAIN, 1.0);
  filter.frequency = MIN_FREQUENCY + current_range * level;

  // Update the filter state
  filter.update();

  // Control led
  if (!zeroStomp.isBypassed()) {
    zeroStomp.setLed(MAX_LED * (min(level * GAIN_LED, 1.0) * (1.0 - MIN_LED) + MIN_LED));
  } else {
    zeroStomp.setLed(0);
  }
}

void updateAudio(int32_t *l, int32_t *r) {
  // Update the state of the envelope (audio is not altered)
  envelope.process(l, r);

  // Process samples through band pass filter biquad
  filter.process(l, r);
}
