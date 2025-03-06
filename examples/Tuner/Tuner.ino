// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Detect.h"

#define SAMPLE_RATE (DEFAULT_SAMPLE_RATE)

Detect detect(DEFAULT_FFT_SIZE, SAMPLE_RATE, DEFAULT_CHANNELS);

void setup(void) {
  // Open Serial
  Serial.begin(115200);
  Serial.println("Zero Stomp - Tuner");

  zeroStomp.setTitle("Chromatic Tuner");
  zeroStomp.setSampleRate(SAMPLE_RATE);

  if (!zeroStomp.begin()) {
    Serial.println("Failed to initiate device");
    while (1) { };
  }
}

float freq;
char buf[10];
const char *notename;

void updateControl(uint32_t samples) {
  // Get detected frequency and convert to string
  freq = detect.getFrequency();
  snprintf(buf, sizeof(buf), "%.0fhz", freq);

  // Clear area
  zeroStomp.display.fillRect(
      0, CONTROL_Y,
      DISPLAY_WIDTH, DISPLAY_HEIGHT - CONTROL_Y,
      0
  );

  // Setup font
  zeroStomp.display.setTextSize(1);
  zeroStomp.display.setTextColor(SSD1306_WHITE);

  // Print frequency
  zeroStomp.display.setCursor(
    (DISPLAY_WIDTH / 2) - (STR_WIDTH(strlen(buf)) / 2),
    CONTROL_Y + (DISPLAY_HEIGHT - CONTROL_Y) / 2 - CHAR_HEIGHT / 2 - 2
  );
  zeroStomp.display.write(buf, strlen(buf));

  // Print note
  notename = detect.getNoteName();
  zeroStomp.display.setCursor(
    (DISPLAY_WIDTH / 2) - (STR_WIDTH(strlen(notename)) / 2),
    CONTROL_Y + (DISPLAY_HEIGHT - CONTROL_Y) / 2 + CHAR_HEIGHT / 2 + 2
  );
  zeroStomp.display.write(notename, strlen(notename));

  // Update display
  zeroStomp.display.display();
}

void updateAudio(int32_t *l, int32_t *r) {
  detect.process(l, r);
}
