// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "effects/Detect.h"
#include "effects/Envelope.h"

#define SAMPLE_RATE (DEFAULT_SAMPLE_RATE)
#define TEXT_PAD (4)
#define TEXT_CENTER (CONTROL_Y + (DISPLAY_HEIGHT - CONTROL_Y) / 2)

Detect detect(DEFAULT_FFT_SIZE, SAMPLE_RATE);
Envelope envelope;

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

float freq = 0.0;
char freq_buf[10];
int notenum = 0;
char notenum_buf[10];
char notename_buf[10];

void updateControl(uint32_t samples) {
  Serial.println(envelope.get());

  // Clear area
  zeroStomp.display.fillRect(
      0, CONTROL_Y,
      DISPLAY_WIDTH, DISPLAY_HEIGHT - CONTROL_Y,
      0
  );

  // Setup font
  zeroStomp.display.setTextSize(1);
  zeroStomp.display.setTextColor(SSD1306_WHITE);

  if (envelope.isActive() && detect.ready()) {
    // Get detected frequency and convert to string
    freq = detect.getFrequency();
    snprintf(freq_buf, sizeof(freq_buf), "%.0fhz", freq);

    // Get midi note number and convert to string
    notenum = detect.getNoteNum();
    snprintf(notenum_buf, sizeof(notenum_buf), "%d", notenum);
    
    // Get note name for frequency
    detect.getNoteName(notename_buf, sizeof(notename_buf));
  }

  if (freq > 0.0) {

    // Print frequency
    zeroStomp.display.setCursor(
      (DISPLAY_WIDTH / 2) - (STR_WIDTH(strlen(freq_buf)) / 2),
      TEXT_CENTER - CHAR_HEIGHT * 3 / 2 - TEXT_PAD
    );
    zeroStomp.display.write(freq_buf, strlen(freq_buf));

    // Print midi note number
    zeroStomp.display.setCursor(
      (DISPLAY_WIDTH / 2) - (STR_WIDTH(strlen(notenum_buf)) / 2),
      TEXT_CENTER - CHAR_HEIGHT / 2
    );
    zeroStomp.display.write(notenum_buf, strlen(notenum_buf));

    // Print note name
    zeroStomp.display.setCursor(
      (DISPLAY_WIDTH / 2) - (STR_WIDTH(strlen(notename_buf)) / 2),
      TEXT_CENTER + CHAR_HEIGHT / 2 + TEXT_PAD
    );
    zeroStomp.display.write(notename_buf, strlen(notename_buf));

  }

  // Update display
  zeroStomp.display.display();

  // Control led
  if (!zeroStomp.isBypassed()) {
    zeroStomp.setLed(envelope.isActive() ? MAX_LED : (MAX_LED >> 1));
  } else {
    zeroStomp.setLed(0);
  }
}

void updateAudio(int32_t *l, int32_t *r) {
  // Check to see if envelope is active
  envelope.process(l, r);
  if (!envelope.isActive()) return;

  // Reset detection if envelope triggered
  if (envelope.didAttack()) detect.flush();

  // Add sample data to detector
  detect.process(l, r);
}
