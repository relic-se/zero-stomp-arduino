// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "LFO.h"
#include "effects/Chorus.h"

#define MIN_VOICES (1)
#define MAX_VOICES (5)

// BUG: Rate is too fast
#define MIN_RATE (0.01)
#define MAX_RATE (0.1)

#define MIN_DEPTH (0.01) // 10ms +/ 5ms
#define MAX_DEPTH (0.1) // 100ms +/- 50ms

#define USE_LFO (1)

#if USE_LFO
#define KNOB_VOICES (0)
#define KNOB_RATE (1)
#define KNOB (KNOB_VOICES)

Chorus effect(MAX_DEPTH * 1.5);
LFO lfo;
#else
Chorus effect(MAX_DEPTH);
#endif

float depth;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Chorus");

  #if USE_LFO
  lfo.setWaveform(lfoWaveformSine);
  #endif

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Chorus"));
  zeroStomp.setLabel(0, F("Level"));
  #if !USE_LFO || KNOB == KNOB_VOICES
  zeroStomp.setLabel(1, F("Voices"));
  #if USE_LFO
  lfo.setRate(MIN_RATE);
  #endif
  #else
  zeroStomp.setLabel(1, F("Rate"));
  effect.setVoices(MAX_VOICES);
  #endif
  zeroStomp.setLabel(2, F("Depth"));
}

void updateControl(uint32_t samples) {
  zeroStomp.setMix(zeroStomp.getValue(0) >> 5); // Only allow 0-127

  #if !USE_LFO || KNOB == KNOB_VOICES
  effect.setVoices(map(zeroStomp.getValue(1), 0, 4096, MIN_VOICES, MAX_VOICES));
  #else
  lfo.setRate(mapFloat(zeroStomp.getValue(1), 0, 4096, MIN_RATE, MAX_RATE));
  #endif

  depth = mapFloat(zeroStomp.getValue(2), 0, 4096, MIN_DEPTH, MAX_DEPTH);

  #if !USE_LFO
  effect.setTime(depth);
  #else
  lfo.setOffset(depth);
  lfo.setScale(depth / 2);
  effect.setTime(lfo.get());
  #endif
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
