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
    _control_timer = 0;

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

int ZeroStomp::getAdcPin(uint8_t index) {
    switch (index) {
        case 1:
            return PIN_ADC_1;
        case 2:
            return PIN_ADC_2;
        case 3:
            return PIN_ADC_EXPR;
        default:
            return PIN_ADC_0;
    }
}

int16_t ZeroStomp::getAdcValue(uint8_t index) {
    return analogRead(getAdcPin(index));
};

void ZeroStomp::update() {
    // TODO: Run audio processing on second core

    if (!_running) {
        return;
    }

    // TODO: Support 24-bit and 32-bit samples

    size_t count = _i2s.read((const uint8_t *)_buffer, _buffer_size) * sizeof(uint32_t) / sizeof(int16_t);
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
    if (_control_timer >= CONTROL_RATE) {
        updateControl();
        _control_timer = 0;
    }
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

bool ZeroStomp::drawTitle(const String &s, bool update) {
    if (!prepareTitle(s.length())) {
        return false;
    }
    
    if (!_display.println(s)) {
        return false;
    }

    if (update) {
        // Update display
        _display.display();
    }

    return true;
};

bool ZeroStomp::drawTitle(const char c[], bool update) {
    if (!prepareTitle(strlen(c))) {
        return false;
    }
    
    if (!_display.println(c)) {
        return false;
    }

    if (update) {
        // Update display
        _display.display();
    }

    return true;
};
