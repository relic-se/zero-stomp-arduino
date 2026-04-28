// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "effects/Filter.h"

Filter::Filter(FilterMode m, float f, float q, int16_t mix, uint8_t channels) :
    Effect(mix, channels), mode(m), frequency(f), Q(q) {
    reset();
};

void Filter::update() {
    float W0 = frequency * global_W_scale;

    if (mode == _mode && (float_equal_or_update(&_W0, W0) & float_equal_or_update(&_Q, Q))) {
        return;
    }
    if (mode != _mode) {
        reset();
    }
    _mode = mode;

    float s, c;
    fast_sincos(W0, &s, &c);

    float alpha = s / (2 * Q);

    float a0, a1, a2, b0, b1, b2;

    a0 = 1 + alpha;
    a1 = -2 * c;
    a2 = 1 - alpha;

    switch (mode) {
        default:
        case LOW_PASS:
            b2 = b0 = (1 - c) * .5;
            b1 = 1 - c;
            break;

        case HIGH_PASS:
            b2 = b0 = (1 + c) * .5;
            b1 = -(1 + c);
            break;

        case BAND_PASS:
            b0 = alpha;
            b1 = 0;
            b2 = -b0;
            break;

        case NOTCH:
            b0 = 1;
            b1 = -2 * c;
            b2 = 1;
    }

    float recip_a0 = 1 / a0;

    assign(
        a1 * recip_a0,
        a2 * recip_a0,
        b0 * recip_a0,
        b1 * recip_a0,
        b2 * recip_a0
    );
};

void Filter::assign(float a1, float a2, float b0, float b1, float b2) {
    _a1 = convertToInt(a1, BIQUAD_SHIFT);
    _a2 = convertToInt(a2, BIQUAD_SHIFT);
    _b0 = convertToInt(b0, BIQUAD_SHIFT);
    _b1 = convertToInt(b1, BIQUAD_SHIFT);
    _b2 = convertToInt(b2, BIQUAD_SHIFT);
};

void Filter::process(int16_t *l, int16_t *r) {
    *l = processSample(*l);
    if (_isStereo) {
        *r = processSample(*r);
    }
};

int16_t Filter::processSample(int16_t input) {
    int32_t x0 = _x[0];
    int32_t x1 = _x[1];
    int32_t y0 = _y[0];
    int32_t y1 = _y[1];

    int16_t output = sat16(_b0 * input + _b1 * _x[0] + _b2 * _x[1] - _a1 * _y[0] - _a2 * _y[1] + (1 << (BIQUAD_SHIFT - 1)), BIQUAD_SHIFT);

    _x[1] = _x[0];
    _x[0] = input;
    _y[1] = _y[0];
    _y[0] = output;

    return output;
};

void Filter::reset() {
    memset(_x, 0, 2 * sizeof(int16_t));
    memset(_y, 0, 2 * sizeof(int16_t));
};
