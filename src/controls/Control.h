// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _CONTROL_H
#define _CONTROL_H

#include "Arduino.h"
#include "display.h"
#include <Adafruit_SSD1306.h>

typedef void (*ControlCallback)(int);

class Control
{
    
public:
    Control(const String &s, int value = 2048);
    Control(const char c[], int value = 2048);

    void setCallback(ControlCallback cb = nullptr);

    int get();
    void set(int value);
    bool update(int value);
    virtual void reset();

    void setTitle(const String &s);
    void setTitle(const char c[]);

    virtual void draw(Adafruit_SSD1306 *display, size_t index, bool update = true);
    void clear(Adafruit_SSD1306 *display, size_t index, bool update = true);

protected:
    int _value;

private:
    int _previous = -1;
    ControlCallback _cb = nullptr;

    const char *_title;
    size_t _title_len = 0;
    bool _title_drawn = false;

};

#endif
