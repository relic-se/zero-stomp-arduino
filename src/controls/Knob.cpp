// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "controls/Knob.h"

void Knob::draw(Adafruit_SSD1306 *display, size_t index, bool update) {
    // Draw title
    Control::draw(display, index, false);

    // Prevent redrawing knob
    /*
    if (_value < 0 || _previous_draw == _value) {
        return;
    }
    */
    _previous_draw = _value;

    // Calculate center position of knob
    int16_t center_x = DISPLAY_WIDTH / CONTROL_COUNT / 2 * ((index % CONTROL_COUNT) * 2 + 1);

    // Clear area
    display->fillRect(
        center_x - KNOB_OUTER_RADIUS, KNOB_Y - KNOB_OUTER_RADIUS,
        KNOB_OUTER_RADIUS * 2, KNOB_OUTER_RADIUS * 2, 0
    );

    // Draw outer circle
    display->drawCircle(
        center_x, KNOB_Y,
        KNOB_OUTER_RADIUS, 1
    );

    // Draw inner circle
    // TODO: Optimization with integer calculation?
    float theta_r = ((float)_value / 4096.0 * -1.5 - 0.25) * PI;
    int16_t knob_x = center_x + (int16_t)((KNOB_OUTER_RADIUS - KNOB_INNER_RADIUS) * sin(theta_r));
    int16_t knob_y = KNOB_Y + (int16_t)((KNOB_OUTER_RADIUS - KNOB_INNER_RADIUS) * cos(theta_r));
    display->drawCircle(
        knob_x, knob_y,
        KNOB_INNER_RADIUS, 1
    );

    if (update) {
        display->display();
    }
};

void Knob::reset() {
    Control::reset();
    _previous_draw = -1;
};
