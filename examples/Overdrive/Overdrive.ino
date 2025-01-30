// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#define MIN_GAIN 0.0
#define MAX_GAIN 40.0

#include "ZeroStomp.h"

float gain;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Overdrive");

  #if !PICO_RP2350
  zeroStomp.setSampleRate(16000);
  zeroStomp.setChannels(1);
  #endif

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }

  zeroStomp.setTitle(F("Overdrive"));

  zeroStomp.setLabel(0, F("Gain"));
  //zeroStomp.setLabel(1, F("Tone"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(size_t samples) {
  gain = dbToLinear(zeroStomp.getValue(0) * (MAX_GAIN - MIN_GAIN) + MIN_GAIN);

  // TODO: Tone EQ
  //zeroStomp.getValue(1)

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2));
}

float applyDrive(float sample) {
  sample *= 0.686306;
  float z = 1.0 + exp(sqrt(fabs(sample)) * -0.75);
  float sample_exp = exp(sample);
  sample *= -1.0;
  sample = (sample_exp - exp(sample * z)) / (sample_exp + exp(sample));
  return sample;
}

void updateAudio(float *l, float *r) {
  *l = applyDrive(*l * gain);
  #if PICO_RP2350
  *r = applyDrive(*r * gain);
  #endif
}
