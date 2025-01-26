// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "config.h"
#include "display.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

ZeroStomp::ZeroStomp(uint32_t sample_rate, uint8_t channels, uint8_t bits_per_sample, size_t buffer_size) :
    _codec(),
    _i2s(INPUT_PULLUP),
    _display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &DISPLAY_SPI, PIN_DISPLAY_DC, PIN_DISPLAY_RESET, PIN_DISPLAY_CS) {
    setSampleRate(sample_rate);
    setChannels(channels);
    setBitsPerSample(bits_per_sample);
    setBufferSize(buffer_size);
};

bool ZeroStomp::begin() {
    if (_running) {
        return false;
    }

    analogReadResolution(12);
    memset((void *)_adc, 0xFF, (KNOB_COUNT + 1) * sizeof(uint16_t));
    memset((void *)_knob, 0xFF, KNOB_COUNT * sizeof(uint16_t));

    // Display
    if (!_display.begin(SSD1306_SWITCHCAPVCC)) {
        return false;
    }

    /// Clear display buffer
    _display.clearDisplay();

    #ifndef NO_SPLASH

    /// Draw splash bitmap
    _display.drawBitmap(
        0, 0,
        splash_bmp, DISPLAY_WIDTH, DISPLAY_HEIGHT, 1
    );

    /// Draw title
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_BLACK);
    _display.setCursor(DISPLAY_WIDTH - STR_WIDTH(TITLE1_LEN) - TITLE_PAD, TITLE_PAD);
    _display.println(TITLE1_STR);
    _display.setCursor(DISPLAY_WIDTH - STR_WIDTH(TITLE2_LEN) - TITLE_PAD, TITLE_PAD + CHAR_HEIGHT + 1);
    _display.println(TITLE2_STR);

    /// Update display
    _display.display();
    delay(SPLASH_DURATION / 3);

    /// Draw animation frame bitmap
    _display.fillRect(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 0
    );
    _display.drawBitmap(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        splash_anim_bmp, SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 1
    );

    /// Update display
    _display.display();
    delay(SPLASH_DURATION / 3);

    /// Redraw splash
    _display.fillRect(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 0
    );
    _display.drawBitmap(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        splash_reset_bmp, SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 1
    );

    /// Update display
    _display.display();
    delay(SPLASH_DURATION / 3);

    /// Clear display
    _display.clearDisplay();
    _display.display();

    #endif

    // MIDI UART
    Serial1.setRX(PIN_UART_RX);
    Serial1.setTX(PIN_UART_TX);
    Serial1.begin(31250);

    // Codec I2C
    I2C_WIRE.setSDA(PIN_I2C_SDA);
    I2C_WIRE.setSCL(PIN_I2C_SCL);
    I2C_WIRE.setClock(I2C_SPEED);
    I2C_WIRE.begin();
    if (!_codec.begin(I2C_WIRE)) {
        return false;
    }

    _active = true;

    // Codec Configuration
    /// Power On
    _codec.enableVREF();
    _codec.enableVMID();

    /// Audio Signal Paths

    //// Enable PGA
    _codec.enableLMIC();
    if (_isStereo) {
        _codec.enableRMIC();
    }
    _codec.enablePgaZeroCross();

    //// Connect from INPUT1 to "n" (aka inverting) input of PGA
    _codec.connectLMN1();
    if (_isStereo) {
        _codec.connectRMN1();
    }

    //// Disable mute on PGA input
    _codec.disableLINMUTE();
    if (_isStereo) {
        _codec.disableRINMUTE();
    }

    //// Set PGA volume
    _codec.setLINVOLDB(0.00);
    if (_isStereo) {
        _codec.setRINVOLDB(0.00);
    }

    //// Set gain of input boost mixer
    _codec.setLMICBOOST(WM8960_MIC_BOOST_GAIN_0DB);
    if (_isStereo) {
        _codec.setRMICBOOST(WM8960_MIC_BOOST_GAIN_0DB);
    }

    //// Connect PGA output to boost mixer
    _codec.connectLMIC2B();
    if (_isStereo) {
        _codec.connectRMIC2B();
    }

    //// Enable boost mixer
    _codec.enableAINL();
    if (_isStereo) {
        _codec.enableAINR();
    }

    //// Connect DAC to output mixer
    _codec.enableLD2LO();
    if (_isStereo) {
        _codec.enableRD2RO();
    }

    /// Digital Interface

    //// Setup Clock
    updateSampleRate();
    updateBitsPerSample();

    //// Enable I2S Peripheral Interface
    _codec.enablePeripheralMode();
    _codec.setALRCGPIO(); // Share ADC LR clock with DAC

    //// Enable ADC
    _codec.enableAdcLeft();
    if (_isStereo) {
        _codec.enableAdcRight();
    }

    //// Enable DAC
    _codec.enableDacLeft();
    if (_isStereo) {
        _codec.enableDacRight();
    }
    _codec.disableLoopBack();

    /// Output

    //// Enable output mixer
    _codec.enableLOMIX();
    if (_isStereo) {
        _codec.enableROMIX();
    }

    //// Headphone amplifier
    _codec.enableHeadphones();
    _codec.disableOUT3MIX(); // Use output caps
    _codec.enableHeadphoneZeroCross();

    //// Update mic bypass, DAC, and headphone output
    updateMix();
    updateLevel();

    // I2S
    _i2s.setBCLK(PIN_I2S_BCLK); // LRCLK is BCLK + 1
    _i2s.setDOUT(PIN_I2S_DOUT);
    _i2s.setDIN(PIN_I2S_DIN);
    _i2s.setFrequency(_sample_rate);
    _i2s.setStereo(_isStereo);
    _i2s.setBitsPerSample(_bits_per_sample);
    _i2s.setBuffers(NUM_DMA_BUFFERS, _buffer_size / sizeof(uint32_t));
    _i2s.begin();

    _buffer = (uint8_t *)malloc(_buffer_size);
    memset((void *)_buffer, 0, _buffer_size);
    _control_timer = _sample_rate / CONTROL_RATE; // Initiate first control update

    _running = true;
    return true;
};

bool ZeroStomp::setSampleRate(uint32_t value) {
    if (_running) {
        return false;
    }
    if (value < 8000 || value > 48000) {
        return false;
    }
    _sample_rate = value;
    if (_active) {
        updateSampleRate();
    }
    return true;
};

bool ZeroStomp::updateSampleRate() {
    if (!_active || _running) {
        return false;
    }

    // Enabled PLL (to generate clock)
    _codec.enablePLL();
    _codec.setSMD(WM8960_PLL_MODE_FRACTIONAL);
    _codec.setCLKSEL(WM8960_CLKSEL_PLL);

    // Setup default scaling
    _codec.setPLLPRESCALE(WM8960_PLLPRESCALE_DIV_2);
    _codec.setSYSCLKDIV(WM8960_SYSCLK_DIV_BY_2);
    _codec.setBCLKDIV(4);
    _codec.setDCLKDIV(WM8960_DCLKDIV_16);

    if (_sample_rate == 8000 || _sample_rate == 12000 || _sample_rate == 16000 || _sample_rate == 24000 || _sample_rate == 32000 || _sample_rate == 48000) {
        // SYSCLK = 12.288 MHz
        // DCLK = 768.0k_hz
        _codec.setPLLN(8);
        _codec.setPLLK(0x31, 0x26, 0xE8);
        _codec.setADCDIV(48000 / _sample_rate);
        _codec.setDACDIV(48000 / _sample_rate);
    } else if (_sample_rate == 11025 || _sample_rate == 22050 || _sample_rate == 44100) {
        // SYSCLK = 11.2896 MHz
        // DCLK = 705.6k_hz
        _codec.setPLLN(7);
        _codec.setPLLK(0x86, 0xC2, 0x26);
        _codec.setADCDIV(44100 / _sample_rate);
        _codec.setDACDIV(44100 / _sample_rate);
    } else {
        return false;
    }

    return true;
};

bool ZeroStomp::setChannels(uint8_t value) {
    if (_running) {
        return false;
    }
    if (value < 1 || value > 2) {
        return false;
    }
    _isStereo = value == 2;
    return true;
};

bool ZeroStomp::setBitsPerSample(uint8_t value) {
    if (_running) {
        return false;
    }
    if (value != 16 && value != 24 && value != 32) {
        return false;
    }
    _bits_per_sample = value;
    if (_active) {
        updateBitsPerSample();
    }
    return true;
};

bool ZeroStomp::updateBitsPerSample() {
    if (!_active || _running) {
        return false;
    }

    // WM8960_WL_16BIT (0), WM8960_WL_24BIT (2), WM8960_WL_32BIT (3)
    uint8_t value = (_bits_per_sample - 16) / 8;
    if (value > 0) {
        value++;
    }

    _codec.setWL(value);
    return true;
};

bool ZeroStomp::setBufferSize(size_t value) { // in bytes
    if (_running || !value || value % sizeof(uint32_t) != 0) {
        return false;
    }
    _buffer_size = value;
    return true;
};

void ZeroStomp::setMix(uint8_t value) {
    _mix = value;
    updateMix();
};

bool ZeroStomp::updateMix() {
    // TODO: Logarithmic
    // TODO: Prevent unnecessary updates

    if (!_active) {
        return false;
    }

    // Mic bypass
    uint8_t volume = 7;
    if (_mix < 255) {
        volume = _mix > 127 ? map(_mix, 128, 255, 0, 7) : 0;
        _codec.enableLB2LO();
        if (_isStereo) {
            _codec.enableRB2RO();
        }
    } else {
        _codec.disableLB2LO();
        if (_isStereo) {
            _codec.disableRB2RO();
        }
    }
    // BUG: Volume lower than dac output
    _codec.setLB2LOVOL(volume);
    if (_isStereo) {
        _codec.setRB2ROVOL(volume);
    }

    // Dac Output
    if (!_mix) {
        _codec.enableDacMute();
    } else {
        _codec.disableDacMute();
    }
    volume = _mix < 128 ? _mix << 1 : 255;
    _codec.setDacLeftDigitalVolume(volume);
    if (_isStereo) {
        _codec.setDacRightDigitalVolume(volume);
    }

    return true;
};

void ZeroStomp::setLevel(uint8_t value) {
    _level = value;
    updateLevel();
};

bool ZeroStomp::updateLevel() {
    // TODO: Logarithmic
    // TODO: Prevent unnecessary updates

    if (!_active) {
        return false;
    }

    _codec.setHeadphoneVolume(map(_level, 0, 255, 47, 127));
    return true;
};

uint16_t ZeroStomp::getValue(uint8_t index) {
    if (!_active || index >= KNOB_COUNT) {
        return 0;
    }

    if (_adc[index] == 0xFFFF) {
        _adc[index] = analogRead(PIN_ADC_0 + index);
        drawKnob(index);
    }

    return _adc[index];
};

uint16_t ZeroStomp::getExpressionValue() {
    if (!_active) {
        return 0;
    }

    if (_adc[KNOB_COUNT] == 0xFFFF) {
        _adc[KNOB_COUNT] = analogRead(PIN_ADC_EXPR);
    }

    return _adc[KNOB_COUNT];
};

void ZeroStomp::update() {
    // TODO: Run audio processing on second core

    if (!_running) {
        return;
    }

    if (_control_timer >= _sample_rate / CONTROL_RATE) {
        // Reset knobs
        memset((void *)_adc, 0xFF, (KNOB_COUNT + 1) * sizeof(uint16_t));

        // Run user code
        updateControl(_control_timer);

        // Update display
        _display.display();

        _control_timer = 0;
    }

    // TODO: Support 24-bit and 32-bit samples

    size_t count = _i2s.read(_buffer, _buffer_size) * sizeof(uint32_t) / sizeof(int16_t);
    int16_t *b = (int16_t *)_buffer;
    size_t index = 0;
    int32_t l, r;
    while (index < count) {
        // Process samples through buffer
        if (_isStereo) {
            l = (int32_t)b[index + 1];
            r = (int32_t)b[index];
        } else {
            l = r = (int32_t)b[index];
        }
        updateAudio(&l, &r);

        // Hard clip samples and update buffer
        if (_isStereo) {
            b[index++] = (int16_t)min(max(r, -32767), 32768);
        }
        b[index++] = (int16_t)min(max(l, -32767), 32768);
    }
    _i2s.write((const uint8_t *)_buffer, count * sizeof(int16_t));

    _control_timer += (_isStereo ? count >> 1 : count);
};

bool ZeroStomp::prepareTitle(size_t len) {
    if (!_active) {
        return false;
    }

    // Clear area
    _display.fillRect(
        0, 0,
        DISPLAY_WIDTH, TITLE_PAD * 2 + CHAR_HEIGHT, 0
    );

    // Draw border
    #if TITLE_BORDER > 0
    _display.fillRect(
        0, TITLE_PAD * 2 + CHAR_HEIGHT,
        DISPLAY_WIDTH, TITLE_BORDER, 1
    );
    #endif

    /// Draw string
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(
        (DISPLAY_WIDTH - STR_WIDTH(len)) / 2,
        TITLE_PAD
    );

    return true;
};

bool ZeroStomp::setTitle(const String &s, bool update) {
    if (!prepareTitle(s.length())) {
        return false;
    }
    
    if (!_display.println(s)) {
        return false;
    }

    if (update) {
        _display.display();
    }

    return true;
};

bool ZeroStomp::setTitle(const char c[], bool update) {
    if (!prepareTitle(strlen(c))) {
        return false;
    }
    
    if (!_display.println(c)) {
        return false;
    }

    if (update) {
        _display.display();
    }

    return true;
};

bool ZeroStomp::prepareKnobLabel(uint8_t index, size_t len) {
    if (!_active) {
        return false;
    }

    index %= KNOB_COUNT;

    // Clear area
    _display.fillRect(
        DISPLAY_WIDTH / KNOB_COUNT * index, LABEL_Y,
        DISPLAY_WIDTH / KNOB_COUNT, CHAR_HEIGHT, 0
    );

    /// Draw string
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(
        (DISPLAY_WIDTH / KNOB_COUNT / 2 * (index * 2 + 1)) - (STR_WIDTH(len) / 2),
        LABEL_Y
    );

    return true;
};

bool ZeroStomp::setLabel(uint8_t index, const String &s, bool update) {
    if (!prepareKnobLabel(index, s.length())) {
        return false;
    }
    
    if (!_display.println(s)) {
        return false;
    }

    if (update) {
        _display.display();
    }

    return true;
};

bool ZeroStomp::setLabel(uint8_t index, const char c[], bool update) {
    if (!prepareKnobLabel(index, strlen(c))) {
        return false;
    }
    
    if (!_display.println(c)) {
        return false;
    }

    if (update) {
        _display.display();
    }

    return true;
};

bool ZeroStomp::drawKnob(uint8_t index) {
    if (!_active) {
        return false;
    }

    index %= KNOB_COUNT;

    // Prevent redrawing knob
    if (_adc[index] != 0xFFFF && _knob[index] != _adc[index]) {
        _knob[index] = _adc[index];

        // Calculate center position of knob
        int16_t center_x = DISPLAY_WIDTH / KNOB_COUNT / 2 * (index * 2 + 1);

        // Clear area
        _display.fillRect(
            center_x - KNOB_OUTER_RADIUS, KNOB_Y - KNOB_OUTER_RADIUS,
            KNOB_OUTER_RADIUS * 2, KNOB_OUTER_RADIUS * 2, 0
        );

        // Draw outer circle
        _display.drawCircle(
            center_x, KNOB_Y,
            KNOB_OUTER_RADIUS, 1
        );

        // Draw inner circle
        // TODO: Optimization with integer calculation?
        float theta_r = ((float)_knob[index] / 4096.0 * -1.5 - 0.25) * PI;
        int16_t knob_x = center_x + (int16_t)((KNOB_OUTER_RADIUS - KNOB_INNER_RADIUS) * sin(theta_r));
        int16_t knob_y = KNOB_Y + (int16_t)((KNOB_OUTER_RADIUS - KNOB_INNER_RADIUS) * cos(theta_r));
        _display.drawCircle(
            knob_x, knob_y,
            KNOB_INNER_RADIUS, 1
        );
    }
    
    return true;
};
