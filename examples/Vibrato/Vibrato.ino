// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Pitch.h"
#include "LFO.h"
#include "controls/Knob.h"
#include "controls/Selector.h"

#define MAX_DEPTH (2.0) // semitones

#define MIN_RATE (0.01)
#define MAX_RATE (1.0)

#define NUM_WAVEFORMS (4)
const LfoWaveform waveforms[NUM_WAVEFORMS] = {
  lfoWaveformTriangle, lfoWaveformSine, lfoWaveformSaw, lfoWaveformSquare,
};
const char *waveform_labels[NUM_WAVEFORMS] = {
  "TRI", "SIN", "SAW", "SQ"
};

Pitch effect;
LFO lfo;
Knob rate("Rate"), depth("Depth"), mix("Mix");
Selector waveform("Shape", NUM_WAVEFORMS, waveform_labels, CONTROL_MIN);

uint8_t waveform_index = 0;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Vibrato");

  zeroStomp.setTitle("Vibrato");
  zeroStomp.addControls(4, &rate, &depth, &mix, &waveform);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  lfo.setRate(rate.getFloat(MIN_RATE, MAX_RATE));

  float lfo_scale = depth.getFloat(MAX_DEPTH);
  lfo.setScale(lfo_scale);

  uint8_t current_waveform_index = waveform.get();
  if (waveform_index != current_waveform_index) {
    waveform_index = current_waveform_index;
    lfo.setWaveform(waveforms[waveform_index]);
  }

  float value = lfo.get();
  effect.setShift(value);

  zeroStomp.setMix(mix.get(255));

  // Control led
  zeroStomp.setLed(!zeroStomp.isBypassed() ? (MAX_LED * (1.0 - (value / 2 + lfo_scale) / MAX_DEPTH)) : 0);
}

void updateAudio(int32_t *l, int32_t *r) {
  effect.process(l, r);
}
