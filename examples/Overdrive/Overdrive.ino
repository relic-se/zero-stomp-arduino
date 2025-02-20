// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "controls/Knob.h"

#define MIN_GAIN (0.0)
#define MAX_GAIN (40.0)

Knob gain("Drive"), level("Level");
float pre_gain, post_gain;

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
  zeroStomp.addControls(2, &gain, &level);
}

void updateControl(uint32_t samples) {
  float current_gain = gain.getFloat(MIN_GAIN, MAX_GAIN);
  pre_gain = dbToLinear(current_gain);
  post_gain = dbToLinear(-current_gain);

  // TODO: Tone EQ

  // Update output level through codec
  zeroStomp.setLevel(level.get(255));
}

float applyDrive(float sample) {
  sample *= 0.686306;
  float z = 1.0 + exp(sqrt(fabs(sample)) * -0.75);
  float sample_exp = exp(sample);
  sample *= -1.0;
  sample = (sample_exp - exp(sample * z)) / (sample_exp + exp(sample));
  return sample;
}

void processSample(int32_t *sample) {
  // Convert sample to float (-1.0 to 1.0)
  float samplef = convert(*sample);

  // Apply pre-gain to signal
  samplef *= pre_gain;

  // Apply overdrive
  samplef = applyDrive(samplef);

  // Apply post-gain to signal
  samplef *= post_gain;

  // Convert back to integer format
  *sample = convert(samplef);
}

void updateAudio(int32_t *l, int32_t *r) {
  processSample(l);
  #if PICO_RP2350
  processSample(r);
  #endif
}
