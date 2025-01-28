// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

// NOTE: Requires RP2350 for floating point calculation

#define MIN_GAIN 0.0
#define MAX_GAIN 20.0

#include "ZeroStomp.h"
#include "ZeroUtils.h"

float gain;

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Overdrive");

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate zeroStomp");
    while (1) { };
  }

  zeroStomp.setTitle(F("Overdrive"));

  zeroStomp.setLabel(0, F("Gain"));
  //zeroStomp.setLabel(1, F("Tone"));
  zeroStomp.setLabel(2, F("Level"));
}

void updateControl(uint32_t samples) {
  gain = dbToLinear(mapFloat(zeroStomp.getValue(0), 0, 4096, MIN_GAIN, MAX_GAIN));

  // TODO: Tone EQ
  //zeroStomp.getValue(1)

  // Update output level through codec
  zeroStomp.setLevel(zeroStomp.getValue(2) >> 4);
}

float applyDrive(float sample) {
  sample *= 0.686306;
  float z = 1.0 + exp(sqrt(fabs(sample)) * -0.75);
  float sample_exp = exp(sample);
  sample *= -1.0;
  sample = (sample_exp - exp(sample * z)) / (sample_exp + exp(sample));
  return sample;
}

void updateAudio(int32_t *l, int32_t *r) {
  // Convert samples to float (-1.0 to 1.0)
  float fl, fr;
  fl = sampleToFloat(*l);
  fr = sampleToFloat(*r);

  // Apply gain to signal
  fl *= gain;
  fr *= gain;

  // Apply overdrive
  fl = applyDrive(fl);
  fr = applyDrive(fr);

  // Convert back to sample format
  *l = sampleFromFloat(fl);
  *r = sampleFromFloat(fr);
}
