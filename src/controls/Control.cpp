// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "controls/Control.h"

Control::Control(const String &s, int value) : _value(value) {
    setTitle(s);
    reset();
};

Control::Control(const char c[], int value) : _value(value) {
    setTitle(c);
    reset();
};

void Control::reset() {
    _previous = -1;
    if (_value >= 0) {
        set(_value);
    }
};

void Control::set(int value) {
    _value = value;
    if (_cb != nullptr) (*_cb)(value);
};

void Control::setCallback(ControlCallback cb) {
    _cb = cb;
};

int Control::get() {
    return _value;
};

bool Control::update(int value) {
    bool result = false;
    if (_previous >= 0 && (
        (_previous == _value) // Actively updating
        || (_previous < _value && value >= _value) // Moved right
        || (_previous > _value && value <= _value)) // Moved left
    ) {
        set(value);
        result = true;
    }
    _previous = value;
    return result;
};

void Control::setTitle(const String &s) {
    _title = s.c_str();
    _title_len = s.length();
};

void Control::setTitle(const char c[]) {
    _title = c;
    _title_len = strlen(c);
};

void Control::draw(Adafruit_SSD1306 *display, size_t index, bool update) {
    if (!_title_len || _title_drawn) return;

    // Clear area
    display->fillRect(
        DISPLAY_WIDTH / CONTROL_COUNT * (index % CONTROL_COUNT), CONTROL_TITLE_Y,
        DISPLAY_WIDTH / CONTROL_COUNT, CHAR_HEIGHT,
        0
    );

    /// Draw string
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(
        (DISPLAY_WIDTH / CONTROL_COUNT / 2 * ((index % CONTROL_COUNT) * 2 + 1)) - (STR_WIDTH(_title_len) / 2),
        CONTROL_TITLE_Y
    );
    if (!display->write(_title, _title_len)) return;

    if (update) {
        display->display();
    }

    _title_drawn = true;
};

void Control::clear(Adafruit_SSD1306 *display, size_t index, bool update) {
    display->fillRect(
        DISPLAY_WIDTH / CONTROL_COUNT * (index % CONTROL_COUNT), CONTROL_Y,
        DISPLAY_WIDTH / CONTROL_COUNT, DISPLAY_HEIGHT - CONTROL_Y,
        0
    );

    if (update) {
        display->display();
    }
    
    _title_drawn = false;
};
