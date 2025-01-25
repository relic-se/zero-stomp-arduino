# Zero Stomp Arduino Library
This library provides a layer of hardware abstraction as well as an assortment of audio tools for the Zero Stomp RP2040/RP2350 guitar digital effects pedal.

Interested in the hardware? Find more details within the Zero Stomp repository, https://github.com/relic-se/zero-stomp/.

## Installation
You can download the library as a zipped archive file (see Code -> Download ZIP) and install it with the Arduino IDE using Sketch -> Include Library -> Add .ZIP Library... and then select the downloaded file.

Alternatively, you can use git to clone this repository within the libraries folder of your local Arduino installation. Here is a general example of that procedure on a linux-based device.

```
cd ~/Arduino/libraries
git clone https://github.com/relic-se/zero-stomp-arduino.git
```

This makes it easier to update this library in the future using the command `git pull` within the library's folder.

## Requirements

### Arduino-Pico

This library relies on the Raspberry Pi Pico Arduino core by Earle F. Philhower, III. This core can be installed via the Arduino Boards Manager or manually. Vwersion 4.4.3+ is required for bidirectional I2S support. Please see the arduino-pico repository for more details, https://github.com/earlephilhower/arduino-pico.

### SparkFun WM8960 Arduino Library

In order to communicate with the audio codec on board the Zero Stomp, the WM8960 by Wolfson Microelectronics, the SparkFun WM8960 arduino library is a prerequisite of this library. This library can be installed via the Arduino Library Manager, but please see the library's repository for more details on installation, https://github.com/sparkfun/SparkFun_WM8960_Arduino_Library.

### Adafruit SSD1306

In order to drive the SSD1306 128x64 display via SPI on board the Zero Stomp, the Adafruit SSD1306 library is required. This library can be installed via the Arduino Library Manager. See more information here, https://github.com/adafruit/Adafruit_SSD1306.
