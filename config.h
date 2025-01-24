// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#ifndef _HARDWARE_H
#define _HARDWARE_H

// UART

#ifndef PIN_UART_TX
#define PIN_UART_TX 0
#endif

#ifndef PIN_UART_RX
#define PIN_UART_RX 1
#endif

// I2S

#ifndef PIN_I2S_BCLK
#define PIN_I2S_BCLK 2
#endif

#ifndef PIN_I2S_LRCLK
#define PIN_I2S_LRCLK 3
#endif

#ifndef PIN_I2S_DOUT
#define PIN_I2S_DOUT 4
#endif

#ifndef PIN_I2S_DIN
#define PIN_I2S_DIN 5
#endif

#define DEFAULT_SAMPLE_RATE 48000
#define DEFAULT_BITS_PER_SAMPLE 16
#define DEFAULT_CHANNELS 2
#define DEFAULT_BUFFER_SIZE 512

#ifndef NUM_DMA_BUFFERS
#define NUM_DMA_BUFFERS 6
#endif

// I2C

#ifndef PIN_I2C_SDA
#define PIN_I2C_SDA 6
#endif

#ifndef PIN_I2C_SCL
#define PIN_I2C_SCL 7
#endif

#ifndef I2C_WIRE
#define I2C_WIRE Wire1
#endif

#ifndef I2C_SPEED
#define I2C_SPEED 1000000 // fast mode plus
#endif

// Switch

#ifndef PIN_LED
#define PIN_LED 8
#endif

#ifndef PIN_SWITCH
#define PIN_SWITCH 9
#endif

#ifndef SHORT_DURATION
#define SHORT_DURATION 400 // ms
#endif

// Display

#ifndef PIN_DISPLAY_RESET
#define PIN_DISPLAY_RESET 10
#endif

#ifndef PIN_DISPLAY_DC
#define PIN_DISPLAY_DC 11
#endif

#ifndef PIN_DISPLAY_CS
#define PIN_DISPLAY_CS 13
#endif

#ifndef PIN_DISPLAY_SCK
#define PIN_DISPLAY_SCK 14
#endif

#ifndef PIN_DISPLAY_TX
#define PIN_DISPLAY_TX 15
#endif

#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH 128
#endif

#ifndef DISPLAY_HEIGHT
#define DISPLAY_HEIGHT 64
#endif

// ADC

#ifndef PIN_ADC_0
#define PIN_ADC_0 26
#endif

#ifndef PIN_ADC_1
#define PIN_ADC_1 27
#endif

#ifndef PIN_ADC_2
#define PIN_ADC_2 28
#endif

#ifndef PIN_ADC_EXPR
#define PIN_ADC_EXPR 29
#endif

// Program

#ifndef CONTROL_RATE
#define CONTROL_RATE 2400 // 20hz at 48Khz
#endif

#endif
