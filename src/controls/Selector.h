// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _SELECTOR_H
#define _SELECTOR_H

#include "Arduino.h"
#include "controls/Control.h"
#include "display.h"

class Selector : public Control
{

public:
    Selector(const String &s, size_t count, const char *items[], int value = 0) : Control(s, value), _count(count), _items(items) { };
    Selector(const char c[], size_t count, const char *items[], int value = 0) : Control(c, value), _count(count), _items(items) { };

    int get() override;

    void draw(Adafruit_SSD1306 *display, size_t index, bool update = true);

protected:
    void reset() override;

private:
    int _previous_draw = -1;
    size_t _count;
    const char **_items;

};

#endif
