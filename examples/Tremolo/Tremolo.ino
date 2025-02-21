// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "LFO.h"
#include "controls/Knob.h"
#include "controls/Selector.h"

#define MIN_RATE (0.01)
#define MAX_RATE (0.5)

#define NUM_WAVEFORMS (4)
const LfoWaveform waveforms[NUM_WAVEFORMS] = {
  lfoWaveformTriangle, lfoWaveformSine, lfoWaveformSaw, lfoWaveformSquare,
};
const char *waveform_labels[NUM_WAVEFORMS] = {
  "TRI", "SIN", "SAW", "SQ"
};

LFO lfo;
Knob depth("Depth"), rate("Speed");
Selector waveform("Shape", NUM_WAVEFORMS, waveform_labels);

uint8_t waveform_index = 0;
int16_t level;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Tremolo");

  zeroStomp.setTitle("Tremolo");
  zeroStomp.addControls(3, &depth, &rate, &waveform);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  float current_depth = depth.getFloat(0.0, 0.5);
  lfo.setOffset(1.0 - current_depth);
  lfo.setScale(current_depth);

  lfo.setRate(rate.getFloat(MIN_RATE, MAX_RATE));

  uint8_t current_waveform_index = waveform.get();
  if (waveform_index != current_waveform_index) {
    waveform_index = current_waveform_index;
    lfo.setWaveform(waveforms[waveform_index]);
  }

  // Level controlled by codec
  level = lfo.get_scaled(0.0, 1.0);
  zeroStomp.setLevel((uint8_t)min(level >> 7, UMAX_VALUE(uint8_t)));

  // Control led
  zeroStomp.setLed(!zeroStomp.isBypassed() ? (MAX_LED * level) : 0);
}

void updateAudio(int32_t *l, int32_t *r) {
  // Level controlled by DSP
  //*l = scale<int16_t>(*l, level);
  //*r = scale<int16_t>(*r, level);
}
