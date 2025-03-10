/*

  GraphicsTest.ino

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
    #include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
    #include <Wire.h>
#endif

#include "./assets/c-base-logo.h"
#include "./assets/c-base-shape.h"

/*
  U8g2lib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM
  consumption Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards. U8x8 Text
  Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.

  This is a page buffer example.
  */

// // builtin pins for hardware SPI
// // https://www.teachmemicro.com/esp32-pinout-diagram-wroom-32/
// #define SPI_BUILTIN_CIPO_PIN 12
// #define SPI_BUILTIN_COPI_PIN 13
// #define SPI_BUILTIN_SCK_PIN 14
// #define SPI_BUILTIN_CS_PIN 15

// // changeable
// #define SPI_RESET_PIN 25
// #define SPI_DC_PIN 26

// // implictly assumes: sck/clk on pin 14, sdin/copi/miso on pin 13 ("hardware SPI")
// U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/SPI_BUILTIN_CS_PIN, /* dc=*/SPI_DC_PIN,
//                                          /* reset=*/SPI_RESET_PIN);

// builtin pins for hardware SPI
// https://www.teachmemicro.com/esp32-pinout-diagram-wroom-32/
#define SPI_BUILTIN_CIPO_PIN 12
#define SPI_BUILTIN_COPI_PIN 13
#define SPI_BUILTIN_SCK_PIN 14
#define SPI_BUILTIN_CS_PIN 15

// changeable
#define SPI_RESET_PIN 25
#define SPI_DC_PIN 26

// implictly assumes: sck/clk on pin 14, sdin/copi/miso on pin 13 ("hardware SPI")
U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI
u8g2(
    U8G2_R0,
    /* sck */ SPI_BUILTIN_SCK_PIN,
    /* data */ SPI_BUILTIN_COPI_PIN,
    /* cs */ SPI_BUILTIN_CS_PIN,
    /* dc */ SPI_DC_PIN,
    /* reset */ SPI_RESET_PIN
);

void u8g2_prepare(void) {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
}

void draw_img(void) {
    // u8g2.setBitmapMode(false /* solid */);
    // note: assumes image is in progmem, use `u8g2.drawXBM` for dynamically generated images
    u8g2.drawXBMP(0, 0, c_base_logo.width, c_base_logo.height, c_base_logo.data);
}

void setup(void) {
    /* U8g2 Project: SSD1306 Test Board */
    pinMode(SPI_BUILTIN_CS_PIN, OUTPUT);
    pinMode(SPI_DC_PIN, OUTPUT);
    digitalWrite(SPI_BUILTIN_CS_PIN, 0);
    digitalWrite(SPI_DC_PIN, 0);

    // https://github.com/olikraus/u8g2/blob/2b75f932b5ef4b4de8edf73e1a690702a35b1976/doc/faq.txt#L35-L36
    // SPI = SPIClass(HSPI);

    u8g2.begin();
}

int loopDurationMs = 1000;

double square(double x) {
    return x * x;
}

void loop(void) {
    static long unsigned lastTime = 0;
    if (lastTime == 0) {
        lastTime = millis();
    }

    static long unsigned delta = millis() - lastTime;
    lastTime = millis();

    u8g2_prepare();
    // u8g2.firstPage();
    u8g2.clearBuffer();

    auto time = millis() / 10;
    double loopPercent = (double)(time % loopDurationMs) / (double)loopDurationMs;
    double speed = 100 * square(sin(loopPercent * PI));

    char speedStr[64];
    snprintf(speedStr, (sizeof speedStr), "%05.2f", speed);
    // std::string speedStr = std::format("{:.2} mm/s", speed);
    // std::string speedStr = std::to_string(speed) + " mm/s";

    // do {
    u8g2.drawXBMP(2, 2, c_base_shape.width, c_base_shape.height, c_base_shape.data);
    // draw_img();
    u8g2.setFont(u8g2_font_6x10_mf);
    u8g2.drawStr(c_base_shape.width + 2 + 16, 8, "Monorail speed");

    u8g2.setFont(u8g2_font_inb24_mn);
    u8g2.drawStr(c_base_shape.width + 2 + 16, 20, speedStr);

    u8g2.setFont(u8g2_font_12x6LED_tf);
    u8g2.drawStr(3 * 64 - 16, 64 - 30, "mm/s");

    u8g2.sendBuffer();

    // delay(100);
}
