// SPDX-FileCopyrightText: Copyright (c) 2025 Cooper Dalrymple
//
// SPDX-License-Identifier: GPLv3

#include "ZeroStomp.h"
#include "display.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

ZeroStomp zeroStomp;

float global_rate_scale = 0.0, global_W_scale = 0.0;
uint8_t global_tick = 0;

void loop() {
    #ifdef SINGLE_CORE
    zeroStomp.update();
    #else
    while (rp2040.fifo.available()) {
        zeroStomp.updateControls(rp2040.fifo.pop());
    }
    #endif
};

#ifndef SINGLE_CORE

bool core1_separate_stack = true;

void setup1() {
    // Do nothing
};

void loop1() {
    zeroStomp.update();
};

#endif

ZeroStomp::ZeroStomp() :
    _codec(),
    _i2s(INPUT_PULLUP),
    display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &DISPLAY_SPI, PIN_DISPLAY_DC, PIN_DISPLAY_RESET, PIN_DISPLAY_CS) {
    setSampleRate(DEFAULT_SAMPLE_RATE);
    setChannels(DEFAULT_CHANNELS);
    setBitsPerSample(BITS_PER_SAMPLE);
    setBufferSize(DEFAULT_BUFFER_SIZE);

    // ADCs
    analogReadResolution(ADC_BITS);

    // Stomp

    /// PWM LED
    analogWriteFreq(LED_FREQUENCY);
    analogWriteResolution(sizeof(uint16_t) * 8);
    pinMode(PIN_LED, OUTPUT);
    analogWrite(PIN_LED, 0);

    /// Switch
    pinMode(PIN_SWITCH, INPUT_PULLUP);
    _switch_value = isBypassed();
    _switch_millis = millis();
};

bool ZeroStomp::begin() {
    if (_running) {
        return false;
    }

    // Display
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        return false;
    }

    /// Clear display buffer
    display.clearDisplay();

    #ifndef NO_SPLASH

    /// Draw splash bitmap
    display.drawBitmap(
        0, 0,
        splash_bmp, DISPLAY_WIDTH, DISPLAY_HEIGHT, 1
    );

    /// Draw title
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(DISPLAY_WIDTH - STR_WIDTH(TITLE1_LEN) - TITLE_PAD, TITLE_PAD);
    display.println(TITLE1_STR);
    display.setCursor(DISPLAY_WIDTH - STR_WIDTH(TITLE2_LEN) - TITLE_PAD, TITLE_PAD + CHAR_HEIGHT + 1);
    display.println(TITLE2_STR);

    /// Update display
    display.display();
    delay(SPLASH_DURATION / 3);

    /// Draw animation frame bitmap
    display.fillRect(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 0
    );
    display.drawBitmap(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        splash_anim_bmp, SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 1
    );

    /// Update display
    display.display();
    delay(SPLASH_DURATION / 3);

    /// Redraw splash
    display.fillRect(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 0
    );
    display.drawBitmap(
        SPLASH_ANIM_X, SPLASH_ANIM_Y,
        splash_reset_bmp, SPLASH_ANIM_WIDTH, SPLASH_ANIM_HEIGHT, 1
    );

    /// Update display
    display.display();
    delay(SPLASH_DURATION / 3);

    /// Clear display
    display.clearDisplay();
    display.display();

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

    _codec.setVSEL(WM8960_VSEL_LOWEST_BIAS_CURRENT);
    _codec.setVROI(WM8960_VROI_500);
    _codec.setVMID(WM8960_VMIDSEL_2X5KOHM);

    /// Audio Signal Paths

    #ifdef USE_PGA

    //// Enable PGA
    _codec.enableLMIC();
    if (_isStereo) {
        _codec.enableRMIC();
    }
    _codec.enablePgaZeroCross();

    //// Connect INPUT1 to "n" (aka inverting) input of PGA
    _codec.connectLMN1();
    if (_isStereo) {
        _codec.connectRMN1();
    }

    //// Connect VMID to "p" (aka non-inverting) input of PGA
    zeroStomp._codec.pgaLeftNonInvSignalSelect(WM8960_PGAL_VMID);
    zeroStomp._codec.pgaRightNonInvSignalSelect(WM8960_PGAL_VMID);

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

    #endif

    //// Set gain of input boost mixer
    _codec.setLMICBOOST(WM8960_MIC_BOOST_GAIN_0DB);
    if (_isStereo) {
        _codec.setRMICBOOST(WM8960_MIC_BOOST_GAIN_0DB);
    }

    //// Connect input boost output to boost mixer
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

    //// Disable unused inputs
    zeroStomp._codec.setLIN2BOOST(WM8960_BOOST_MIXER_GAIN_MUTE);
    zeroStomp._codec.setRIN2BOOST(WM8960_BOOST_MIXER_GAIN_MUTE);

    zeroStomp._codec.setLIN3BOOST(WM8960_BOOST_MIXER_GAIN_MUTE);
    zeroStomp._codec.setRIN3BOOST(WM8960_BOOST_MIXER_GAIN_MUTE);

    zeroStomp._codec.disableLI2LO();
    zeroStomp._codec.disableRI2RO();

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

    _codec.enableDac6dbAttenuation(); // Improves continuity with bypass output level
    _codec.setDacDeEmphasis(WM8960_DEEMPH_48K); // Reduces some noise
    _codec.enableDacSlopingStopbandFilter(); // Minimal effect on output

    _codec.enableDacSoftMute();
    _codec.disableDacSlowSoftMute();

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
    _i2s.setFrequency(_sampleRate);
    _i2s.setStereo(_isStereo);
    _i2s.setBitsPerSample(_bits_per_sample);
    _i2s.setBuffers(NUM_DMA_BUFFERS, _buffer_size / sizeof(uint32_t));
    _i2s.begin();

    _buffer = (uint32_t *)malloc(_buffer_size);
    memset((void *)_buffer, 0, _buffer_size);
    _control_timer = _sampleRate / CONTROL_RATE; // Initiate first control update

    // Allow filter and LFO updates to run during setup
    control_tick(_sampleRate, _control_timer);
    _running = true;

    // Draw screen
    redraw();

    return true;
};

bool ZeroStomp::setSampleRate(uint32_t value) {
    if (_running) {
        return false;
    }
    if (value < 8000 || value > 48000) {
        return false;
    }
    _sampleRate = value;
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

    uint16_t div_base;
    if (_sampleRate == 8000 || _sampleRate == 12000 || _sampleRate == 16000 || _sampleRate == 24000 || _sampleRate == 32000 || _sampleRate == 48000) {
        // SYSCLK = 12.288 MHz
        // DCLK = 768.0k_hz
        _codec.setPLLN(8);
        _codec.setPLLK(0x31, 0x26, 0xE8);
        div_base = 48000;
    } else if (_sampleRate == 11025 || _sampleRate == 22050 || _sampleRate == 44100) {
        // SYSCLK = 11.2896 MHz
        // DCLK = 705.6k_hz
        _codec.setPLLN(7);
        _codec.setPLLK(0x86, 0xC2, 0x26);
        div_base = 44100;
    } else {
        return false;
    }

    uint8_t div = div_base * 2 / _sampleRate;
    switch (div) {
        case 2:
        default:
            div = 0;
            break;
        case 3:
            div = 1;
            break;
        case 4:
            div = 2;
            break;
        case 6:
            div = 3;
            break;
        case 8:
            div = 4;
            break;
        case 11:
            div = 5;
            break;
        case 12:
            div = 6;
            break;
    }
    _codec.setADCDIV(div);
    _codec.setDACDIV(div);

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

    uint8_t mix = !isBypassed() ? _mix : 0;

    // Mic bypass
    uint8_t volume = 7;
    if (mix < 255) {
        volume = mix > 127 ? map(mix, 128, 255, 0, 7) : 0;
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
    if (!mix) {
        _codec.enableDacMute();
    } else {
        _codec.disableDacMute();
    }
    volume = mix < 128 ? mix << 1 : 255;
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

int ZeroStomp::getExpression() {
    if (!_active) {
        return 0;
    }

    if (_adc_expr < 0) {
        _adc_expr = analogRead(PIN_ADC_EXPR);
    }

    return _adc_expr;
};

int ZeroStomp::getExpression(int max_value) {
    return getExpression(0, max_value);
};

int ZeroStomp::getExpression(int min_value, int max_value) {
    return mapControl(getExpression(), min_value, max_value);
};

float ZeroStomp::getExpressionFloat() {
    return getExpressionFloat(0.0, 1.0);
};

float ZeroStomp::getExpressionFloat(float max_value) {
    return getExpressionFloat(0.0, max_value);
};

float ZeroStomp::getExpressionFloat(float min_value, float max_value) {
    return mapControlFloat(getExpression(), min_value, max_value);
};

void ZeroStomp::update() {
    if (!_running) {
        return;
    }

    if (_control_timer >= _sampleRate / CONTROL_RATE) {
        // Update sensors and control logic
        #ifndef SINGLE_CORE
        // Trigger on core0
        rp2040.fifo.push(_control_timer);
        #else
        updateControls(_control_timer);
        #endif

        _control_timer = 0;
    }

    // Fill buffers
    uint8_t buffer_count = NUM_DMA_BUFFERS;
    size_t count, index;
    int32_t l, r;
    while (buffer_count-- && _i2s.available()) {
        // Read single buffer
        count = _i2s.read((uint8_t *)_buffer, _buffer_size);
        index = 0;
        while (index < count) {

            // Get samples from buffer
            #if BITS_PER_SAMPLE == 16
            l = (int16_t)((_buffer[index] >> 16) & 0xffff);
            r = (int16_t)((_buffer[index] >> 0) & 0xffff);
            #else
            l = (int32_t)_buffer[index];
            r = (int32_t)_buffer[index + 1];
            #endif

            #if BITS_PER_SAMPLE == 24
            // 24-bit samples are read right-aligned, so left-align them to keep the binary point between 33.32
            l <<= 8;
            r <<= 8;
            #endif

            // Process samples through user code
            if (_isStereo) {
                updateAudio(&l, &r);
            } else {
                updateAudio(&l, &l);
                updateAudio(&r, &r);
            }

            // Apply hard clip
            // BUG: Distortion occurs when using full max value
            l = clip(l, SAMPLE_MAX_VALUE >> 1);
            r = clip(r, SAMPLE_MAX_VALUE >> 1);

            // Update buffer
            #if BITS_PER_SAMPLE == 16
            _buffer[index] = (uint32_t)(((int16_t)l << 16) | ((int16_t)r & 0xffff));
            #else
            _buffer[index] = (uint32_t)l;
            _buffer[index + 1] = (uint32_t)r;
            #endif

            // Increment to next 32-bit word
            #if BITS_PER_SAMPLE == 16
            index++;
            #else
            index += 2;
            #endif
        }
        _i2s.write((const uint8_t *)_buffer, count * sizeof(int32_t));
    }

    #if BITS_PER_SAMPLE == 16
    _control_timer += count << (1 - _isStereo);
    #else
    _control_timer += count >> _isStereo;
    #endif
};

void ZeroStomp::updateControls(uint32_t samples) {
    // Update values needed for processing filters and LFOs
    control_tick(_sampleRate, samples);

    // Update switch, led, and bypass
    bool current_value = isBypassed();
    if (_led_control) {
        analogWrite(PIN_LED, (uint16_t)!current_value * MAX_LED);
    }
    if (_switch_value != current_value) {
        _switch_value = current_value;
        if (_bypass_change_cb != nullptr) (*_bypass_change_cb)(_switch_value);

        unsigned long current_millis = millis();
        if (current_millis - _switch_millis < SWITCH_DURATION) {
            _switch_count++;
            if (_click_cb != nullptr) {
                (*_click_cb)(_switch_count);
            } else if (_switch_count == 1) {
                nextPage();
            }
        } else {
            _switch_count = 0;
        }
        _switch_millis = current_millis;

        updateMix();
    }

    // Update controls on active page with ADC values
    for (size_t i = 0; i < getPageControlCount(); i++) {
        Control *control = _controls[i + _page * CONTROL_COUNT];
        if (control->update(analogRead(PIN_ADC_0 + i))) {
            control->draw(&display, i, false);
        }
    }

    // Reset caching of expression value
    _adc_expr = -1;

    // Run user control code
    updateControl(samples);

    // Update display
    display.display();
};

void ZeroStomp::setTitle(const String &s, bool update) {
    _title = s.c_str();
    _title_len = s.length();
    drawTitle(update);
};

void ZeroStomp::setTitle(const char c[], bool update) {
    _title = c;
    _title_len = strlen(c);
    drawTitle(update);
};

bool ZeroStomp::drawTitle(bool update) {
    if (!_active) {
        return false;
    }

    // Clear area
    display.fillRect(
        0, 0,
        DISPLAY_WIDTH, TITLE_PAD * 2 + CHAR_HEIGHT, 0
    );

    // Draw border
    #if TITLE_BORDER > 0
    display.fillRect(
        0, TITLE_PAD * 2 + CHAR_HEIGHT,
        DISPLAY_WIDTH, TITLE_BORDER, 1
    );
    #endif

    /// Draw string
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(
        (DISPLAY_WIDTH - STR_WIDTH(_title_len)) / 2,
        TITLE_PAD
    );

    if (!display.write(_title, _title_len)) {
        return false;
    }

    if (update) {
        display.display();
    }

    return true;
};

bool ZeroStomp::isBypassed() {
    return digitalRead(PIN_SWITCH) == HIGH;
};

void ZeroStomp::setBypassChange(BypassChangeCallback cb) {
    _bypass_change_cb = cb;
};

void ZeroStomp::setClick(ClickCallback cb) {
    _click_cb = cb;
};

uint16_t ZeroStomp::getLed() {
    return _led_value;
};

void ZeroStomp::setLed(uint16_t value) {
    _led_control = false;
    _led_value = value;
    analogWrite(PIN_LED, value);
};

bool ZeroStomp::addControl(Control *control) {
    if (_num_controls >= MAX_CONTROLS) return false;
    _controls[_num_controls++] = control;

    // Draw control if visible
    if (getPageCount() == _page + 1) {
        control->set(analogRead(PIN_ADC_0 + ((_num_controls - 1) % CONTROL_COUNT)));
        control->draw(&display, _num_controls - 1, true);
    }

    return true;
};

bool ZeroStomp::addControls(int count, ...) {
    bool result = true;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count && result; i++) {
        result = addControl(va_arg(args, Control*));
    }
    va_end(args);
    return result;
};

size_t ZeroStomp::getPageCount() {
    return (max(_num_controls - 1, 0) / CONTROL_COUNT) + 1;
};

size_t ZeroStomp::getPage() {
    return _page;
};

size_t ZeroStomp::getPageControlCount() {
    return min(max(_num_controls - _page * CONTROL_COUNT, 0), CONTROL_COUNT);
};

void ZeroStomp::previousPage(bool update) {
    if (getPageCount() <= 1) return;
    clearPage(false);
    if (_page == 0) {
        _page = getPageCount() - 1;
    } else {
        _page--;
    }
    drawPageTitle(false);
    drawPage(update);
};

void ZeroStomp::nextPage(bool update) {
    if (getPageCount() <= 1) return;
    clearPage(false);
    _page = (_page + 1) % getPageCount();
    drawPageTitle(false);
    drawPage(update);
};

bool ZeroStomp::clearPage(bool update) {
    if (!_active) {
        return false;
    }

    for (size_t i = 0; i < getPageControlCount(); i++) {
        Control *control = _controls[i + _page * CONTROL_COUNT];
        control->clear(&display, i, false);
        control->reset();
    }

    if (update) {
        display.display();
    }

    return true;
};

bool ZeroStomp::drawPage(bool update) {
    if (!_active) {
        return false;
    }
    
    for (size_t i = 0; i < getPageControlCount(); i++) {
        _controls[i + _page * CONTROL_COUNT]->draw(&display, i, false);
    }

    if (update) {
        display.display();
    }

    return true;
};

bool ZeroStomp::drawPageTitle(bool update) {
    if (!_active || _num_controls <= CONTROL_COUNT) {
        return false;
    }

    char text[3] = { // 48 = '0', 49 = '1'
        (char)(49 + _page),
        '/',
        (char)(48 + (char)getPageCount()),
    };

    // Clear area
    display.fillRect(
        DISPLAY_WIDTH - TITLE_PAD - STR_WIDTH(3), TITLE_PAD,
        STR_WIDTH(3), CHAR_HEIGHT,
        0
    );

    /// Draw string
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(
        DISPLAY_WIDTH - TITLE_PAD - STR_WIDTH(3),
        TITLE_PAD
    );

    if (!display.write(text, 3)) {
        return false;
    }

    if (update) {
        display.display();
    }

    return true;
};

void ZeroStomp::redraw() {
    clearPage(false);
    drawTitle(false);
    drawPageTitle(false);
    drawPage(false);
    display.display();
};
