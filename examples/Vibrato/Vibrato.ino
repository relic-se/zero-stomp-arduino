// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Pitch.h"
#include "LFO.h"

#define MAX_DEPTH (2.0) // semitones

// BUG: Rate is too fast
#define MIN_SPEED 0.01
#define MAX_SPEED 0.2

Pitch effect;
LFO lfo;

uint8_t waveform_index = 0;
#define NUM_WAVEFORMS 4

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Vibrato");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Vibrato"));

  zeroStomp.setLabel(0, F("Rate"));
  zeroStomp.setLabel(1, F("Depth"));
  zeroStomp.setLabel(2, F("Shape"));
}

void updateControl(uint32_t samples) {
  lfo.setRate(mapFloat(zeroStomp.getValue(0), 0, 4096, MIN_SPEED, MAX_SPEED));
  lfo.setScale(mapFloat(zeroStomp.getValue(1), 0, 4096, 0.0, MAX_DEPTH));

  uint8_t current_waveform_index = map(zeroStomp.getValue(2), 0, 4096, 0, NUM_WAVEFORMS);
  if (waveform_index != current_waveform_index) {
    waveform_index = current_waveform_index;
    LfoWaveform waveform;
    switch (waveform_index) {
      case 0:
      default:
        waveform = lfoWaveformTriangle;
        break;
      case 1:
        waveform = lfoWaveformSine;
        break;
      case 2:
        waveform = lfoWaveformSaw;
        break;
      case 3:
        waveform = lfoWaveformSquare;
        break;
    }
    lfo.setWaveform(waveform);
  }

  effect.setShift(lfo.get());
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
