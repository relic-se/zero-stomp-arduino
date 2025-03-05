// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Filter.h"
#include "LFO.h"
#include "effects/Envelope.h"
#include "controls/Knob.h"
#include "controls/Selector.h"

#define MIN_FREQUENCY (20) // hz
#define MAX_FREQUENCY (20000) // hz

#define MIN_Q (0.7071067811865475)
#define MAX_Q (8.0)

const char *mode_labels[FILTER_MODES] = {
  "LP", "HP", "BP", "Ntc"
};

#define MIN_RATE (0.01)
#define MAX_RATE (0.2)

#define NUM_WAVEFORMS (4)
const LfoWaveform waveforms[NUM_WAVEFORMS] = {
  lfoWaveformTriangle, lfoWaveformSine, lfoWaveformSaw, lfoWaveformSquare,
};
const char *waveform_labels[NUM_WAVEFORMS] = {
  "TRI", "SIN", "SAW", "SQ"
};

Filter filter;
LFO lfo;
Envelope envelope;

Knob frequency("Freq"), resonance("Q");
Selector mode("Mode", FILTER_MODES, mode_labels);
Knob lfoRate("Rate", CONTROL_MIN), lfoDepth("Depth", CONTROL_MIN);
Selector lfoWaveform("Shape", NUM_WAVEFORMS, waveform_labels, CONTROL_MIN);

int waveform_index = -1;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Filter");
  
  #if !PICO_RP2350
  zeroStomp.setSampleRate(32000);
  #endif

  zeroStomp.setTitle("Filter");
  zeroStomp.addControls(3, &frequency, &resonance, &mode);
  zeroStomp.addControls(3, &lfoRate, &lfoDepth, &lfoWaveform);
  // TODO: Envelope controls; retrigger, speed, etc

  envelope.setAttackCallback(lfo.retrigger);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

void updateControl(uint32_t samples) {
  // Frequency
  float freq = mapControlFloat(frequency.get() + zeroStomp.getExpression(), MIN_FREQUENCY, MAX_FREQUENCY);
  lfo.setOffset(freq);

  // LFO
  lfo.setRate(lfoRate.getFloat(MIN_RATE, MAX_RATE));
  lfo.setScale(min(freq - MIN_FREQUENCY, MAX_FREQUENCY - freq) * lfoDepth.getFloat());

  int current_waveform_index = lfoWaveform.get();
  if (waveform_index != current_waveform_index) {
    waveform_index = current_waveform_index;
    lfo.setWaveform(waveforms[waveform_index]);
  }
  
  // Apply frequency from LFO
  filter.frequency = min(max(lfo.get(), MIN_FREQUENCY), MAX_FREQUENCY);

  // Resonance
  filter.Q = resonance.getFloat(MIN_Q, MAX_Q);

  // Mode
  filter.mode = (FilterMode)mode.get();

  // Update the filter state
  filter.update();
}

void updateAudio(int32_t *l, int32_t *r) {
  // Update envelope
  envelope.process(l, r);

  // Process samples through filter biquad
  filter.process(l, r);
}
