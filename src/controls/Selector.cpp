// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "controls/Selector.h"
#include "ZeroStomp.h"

int Selector::get() {
    return Control::get((int)_count);
};

void Selector::draw(Adafruit_SSD1306 *display, size_t index, bool update) {
    // Draw title
    Control::draw(display, index, false);

    int value = get();

    // Prevent redrawing knob
    if (_previous_draw == value) return;
    _previous_draw = value;

    // Calculate center position of knob
    int16_t center_x = DISPLAY_WIDTH / CONTROL_COUNT / 2 * ((index % CONTROL_COUNT) * 2 + 1);

    // Clear area
    display->fillRect(
        center_x - SELECTOR_OUTER_RADIUS, KNOB_Y - SELECTOR_OUTER_RADIUS,
        SELECTOR_OUTER_RADIUS * 2, SELECTOR_OUTER_RADIUS * 2, 0
    );

    // Draw dots
    float theta_r;
    int16_t radius, dot_x, dot_y;
    for (size_t i = 0; i < _count; i++) {
        theta_r = mapFloat(i, 0, _count - 1, -0.25 * PI, -1.75 * PI);
        dot_x = center_x + (int16_t)(SELECTOR_OUTER_RADIUS * sin(theta_r));
        dot_y = KNOB_Y + (int16_t)(SELECTOR_OUTER_RADIUS * cos(theta_r));
        if (i == value) {
            // Draw selected dot
            display->drawCircle(
                dot_x, dot_y,
                SELECTOR_INNER_RADIUS, 1
            );
        } else {
            display->drawPixel(
                dot_x, dot_y,
                1
            );
        }
    }

    // Draw Text
    const char *text = _items[value];
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(
        center_x - (STR_WIDTH(strlen(text)) / 2),
        KNOB_Y - CHAR_HEIGHT / 2
    );
    display->write(text, strlen(text));

    if (update) {
        display->display();
    }
};

void Selector::reset() {
    Control::reset();
    _previous_draw = -1;
};
