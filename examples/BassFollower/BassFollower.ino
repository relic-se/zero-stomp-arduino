// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Filter.h"
#include "effects/Pitch.h"
#include "controls/Knob.h"

#define MIN_FREQUENCY (100)
#define MAX_FREQUENCY (1000)

Filter filter;
Pitch pitch;
Knob frequency("Filter"), mix("Mix"), level("Level");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Bass Follower");

  zeroStomp.setTitle("Bass Follower");
  zeroStomp.addControls(3, &frequency, &mix, &level);

  filter.Q = 1.5;
  filter.mode = FilterMode::LOW_PASS;

  pitch.setShift(-12.0);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(size_t samples) {
  // Set filter frequency
  filter.frequency = mapControlFloat(frequency.get(), MIN_FREQUENCY, MAX_FREQUENCY, 2.0);
  filter.update();

  // Update mix and output level through codec
  zeroStomp.setMix(mix.get(255));
  zeroStomp.setLevel(level.get(255));
}

void updateAudio(int16_t *l, int16_t *r) {
  filter.process(l, r);
  pitch.process(l, r);
}
