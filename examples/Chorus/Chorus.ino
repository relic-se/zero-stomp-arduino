// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "LFO.h"
#include "effects/Chorus.h"
#include "controls/Knob.h"

#define MIN_VOICES (1)
#define MAX_VOICES (5)

// BUG: Rate is too fast
#define MIN_RATE (0.01)
#define MAX_RATE (0.1)

#define MIN_DEPTH (0.01) // 10ms +/ 5ms
#define MAX_DEPTH (0.1) // 100ms +/- 50ms

#define MAX_LFO_DEPTH (0.5)

#define KNOB_VOICES (0)
#define KNOB_RATE (1)
#define KNOB (KNOB_VOICES)

Chorus effect(MAX_DEPTH * (1.0 + MAX_LFO_DEPTH));
LFO lfo;
Knob level("Level"), voices("Voices"), depth("Depth");
Knob lfoRate("LFO Rate"), lfoDepth("LFO Depth");

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Chorus");

  lfo.setWaveform(lfoWaveformSine);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Chorus"));
  zeroStomp.addControls(3, &level, &voices, &depth);
  zeroStomp.addControls(2, &lfoRate, &lfoDepth);
}

void updateControl(uint32_t samples) {
  zeroStomp.setMix(level.get(127));

  effect.setVoices(voices.get(MIN_VOICES, MAX_VOICES));
  
  float current_depth = depth.getFloat(MIN_DEPTH, MAX_DEPTH);
  lfo.setOffset(current_depth);
  lfo.setRate(lfoRate.getFloat(MIN_RATE, MAX_RATE));
  lfo.setScale(current_depth * lfoDepth.getFloat(MAX_LFO_DEPTH));
  effect.setTime(lfo.get());

  // Control led
  zeroStomp.setLed(!zeroStomp.isBypassed() ? (MAX_LED * (1.0 - (lfo.get() - current_depth / 2) / MAX_DEPTH)) : 0);
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
