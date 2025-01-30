// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "LFO.h"

// BUG: Rate is too fast
#define MIN_SPEED 0.01
#define MAX_SPEED 0.2

LFO lfo;

uint8_t waveform_index = 0;
#define NUM_WAVEFORMS 4

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Loopback");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Loopback"));

  zeroStomp.setLabel(0, F("Depth"));
  zeroStomp.setLabel(1, F("Speed"));
  zeroStomp.setLabel(2, F("Shape"));

  lfo.setOffset(1.0);
  lfo.setScale(0.0);
}

void updateControl(size_t samples) {
  float depth = zeroStomp.getValue(0) * 0.5;
  lfo.setOffset(1.0 - depth);
  lfo.setScale(depth);

  lfo.setRate(zeroStomp.getValue(1) * (MAX_SPEED - MIN_SPEED) + MIN_SPEED);

  uint8_t current_waveform_index = min(floor(zeroStomp.getValue(2) * NUM_WAVEFORMS), NUM_WAVEFORMS - 1);
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

  // Level controlled by codec
  zeroStomp.setLevel(lfo.get());
}

void updateAudio(float *l, float *r) {
  // Level controlled by DSP
  //float level = lfo.get();
  //*l *= level;
  //*r *= level;
}
