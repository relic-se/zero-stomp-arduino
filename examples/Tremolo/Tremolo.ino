// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "ZeroLFO.h"
#include "ZeroUtils.h"

// BUG: Rate is too fast
#define MIN_SPEED 0.01
#define MAX_SPEED 0.2

ZeroLFO lfo;

uint8_t waveform_index = 0;
#define NUM_WAVEFORMS 4

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Loopback");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate zeroStomp");
    while (1) { };
  }

  zeroStomp.setTitle(F("Loopback"));

  zeroStomp.setLabel(0, F("Depth"));
  zeroStomp.setLabel(1, F("Speed"));
  zeroStomp.setLabel(2, F("Shape"));

  lfo.setOffset(1.0);
  lfo.setScale(0.0);
}

void updateControl(uint32_t samples) {
  float depth = mapFloat(zeroStomp.getValue(0), 0, 4096, 0.0, 0.5);
  lfo.setOffset(1.0 - depth);
  lfo.setScale(depth);

  lfo.setRate(mapFloat(zeroStomp.getValue(1), 0, 4096, MIN_SPEED, MAX_SPEED));

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
  

  // Level controlled by codec
  zeroStomp.setLevel((uint8_t)min(lfo.get_scaled(0.0, 1.0) >> 7, (1 << 8) - 1));
}

void updateAudio(int32_t *l, int32_t *r) {
  // Level controlled by DSP
  //int32_t level = lfo.get_scaled(0.0, 1.0);
  //*l = applyVolume(*l, level);
  //*r = applyVolume(*r, level);
}
