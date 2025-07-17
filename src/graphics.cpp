#include "graphics.h"
#include "configuration.h"
#if ENABLE_GRAPHICS

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#define SDA_PIN 5
#define SCL_PIN 6

#define BUGGY_OFFSET 0 // XXX: needed on some broken devices

char DEBUG_BUFFER[100];
char DEBUG_BUFFER2[100];

U8G2_SSD1306_72X40_ER_F_HW_I2C
u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE); // EastRising 0.42" OLED
//
void printText(uint8_t bufid, const char *text) {
  strlcpy(bufid == 1 ? DEBUG_BUFFER : DEBUG_BUFFER2, text, 100);
}

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_crox4tb_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
#if FLIP_SCREEN
  u8g2.setFontDirection(2);
#else
  u8g2.setFontDirection(0);
#endif
}

void graphicsSetup(void) {
  Wire.begin(SDA_PIN, SCL_PIN);
  DEBUG_BUFFER[0] = 0;
  DEBUG_BUFFER2[0] = 0;
  u8g2.begin();
}

#define HEIGHT 40
#define WIDTH 72

void graphicsLoop(void) {
  // picture loop
  u8g2.clearBuffer();
  u8g2_prepare();
  /**
int n = millis() / 10;
if (n > WIDTH && int(n / WIDTH) % 2) {
  n = n % WIDTH;
  u8g2.drawLine(n, 0, n, HEIGHT);
} else {
  n = n % WIDTH;
  u8g2.drawLine(WIDTH - n, 0, WIDTH - n, HEIGHT);
}

  // draw a black background in the center
  u8g2.drawBox(0, 20, 72, 10);
  u8g2.setDrawColor(0);
*/
#if FLIP_SCREEN
  u8g2.drawStr(WIDTH, HEIGHT + BUGGY_OFFSET, DEBUG_BUFFER);
  u8g2.drawStr(WIDTH, (HEIGHT / 2) + BUGGY_OFFSET, DEBUG_BUFFER2);
#else
  u8g2.drawStr(0, 0, DEBUG_BUFFER);
  u8g2.drawStr(5, 20, DEBUG_BUFFER2);
#endif
  //  u8g2.drawStr(0, 3, "12345678910ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  u8g2.sendBuffer();
}

void shutdownGraphics() {
  u8g2.clearDisplay();
  u8g2.setPowerSave(1); // Put display in power-saving mode
}
#else
void printText(uint8_t, const char *) {}
void graphicsSetup() {};
void graphicsLoop() {};
void shutdownGraphics() {};
#endif
