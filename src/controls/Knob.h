// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _ZERO_CHORUS_H
#define _ZERO_CHORUS_H

#include "Arduino.h"
#include "controls/Control.h"
#include "display.h"

class Knob : public Control
{

public:
    Knob(const String &s, int value = 2048) : Control(s, value) { };
    Knob(const char c[], int value = 2048) : Control(c, value) { };

    void draw(Adafruit_SSD1306 *display, size_t index, bool update = true);

protected:
    void reset() override;

private:
    int _previous_draw = -1;

};

#endif
