// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _CONTROL_H
#define _CONTROL_H

#include "Arduino.h"
#include "display.h"
#include <Adafruit_SSD1306.h>

#define CONTROL_MIN (0)
#define CONTROL_MAX (4096)
#define CONTROL_MID (CONTROL_MAX / 2)

typedef void (*ControlCallback)(int);

class Control
{
    
public:
    Control(const String &s, int value = CONTROL_MID);
    Control(const char c[], int value = CONTROL_MID);

    void setCallback(ControlCallback cb = nullptr);

    int get();
    int get(int max_value);
    int get(int min_value, int max_value);

    float getFloat();
    float getFloat(float max_value);
    float getFloat(float min_value, float max_value);

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

static int mapControl(int value, int min_value, int max_value) {
    return map(min(max(value, CONTROL_MIN), CONTROL_MAX), CONTROL_MIN, CONTROL_MAX, min_value, max_value);
};

static float mapControlFloat(int value, float min_value, float max_value) {
    return mapFloat(min(max(value, CONTROL_MIN), CONTROL_MAX), CONTROL_MIN, CONTROL_MAX, min_value, max_value);
};

#endif
