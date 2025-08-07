#include "configuration.h"
#include <stdint.h>
#if ENABLE_GRAPHICS
#include "graphics.h"
#include "icons.h"

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#define SDA_PIN 5
#define SCL_PIN 6

#define HEIGHT 40
#define WIDTH 72
#if BUGGY_DISPLAY
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);
#else
#define BUGGY_OFFSET_X 0
#define BUGGY_OFFSET_Y 0
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
#endif

bool dirty = true;
bool inverted = false;
bool currently_inverted = false;
char DEBUG_BUFFER[100];
// char DEBUG_BUFFER2[100];

bool icon_statuses[icons_count] = {0};

void setIconStatus(int index, bool enabled) {
  if (enabled != icon_statuses[index]) {
    icon_statuses[index] = enabled;
    dirty = true;
  }
}

#if FLIP_SCREEN
void drawBitmap(int x, int y, const Bitmap *bitmap) {
  if (!bitmap || !bitmap->data)
    return;

  int width = bitmap->width;
  int height = bitmap->height;
  int bytes_per_row = (width + 7) / 8;

  // For direction 2 (upside down), we need to:
  // 1. Start from the position which will be the bottom-right corner of the
  // flipped icon
  // 2. Draw pixels going backwards

  for (int py = 0; py < height; py++) {
    for (int px = 0; px < width; px++) {
      // Calculate byte and bit position in the bitmap data
      int byte_idx = py * bytes_per_row + px / 8;
      int bit_idx = px % 8;

      // Check if the bit is set in the bitmap
      int bit_set = (bitmap->data[byte_idx] & (1 << bit_idx)) != 0;

      if (bit_set) {
        // Draw with flipped coordinates
        u8g2.drawPixel(x - px, y - py);
      }
    }
  }
}
#else
static void drawBitmap(int x, int y, const Bitmap *bitmap) {
  if (!bitmap || !bitmap->data)
    return;

  // Draw bitmap using U8G2's XBM drawing function
  u8g2.drawXBM(x, y, bitmap->width, bitmap->height, bitmap->data);
}
#endif

void printText(uint8_t bufid, const char *text) {
  dirty = true;
  strlcpy(DEBUG_BUFFER, text, 100);
}

void u8g2_prepare(void) {
  u8g2.clearBuffer();
  if (inverted) {
    // fill the screen white
    u8g2.setDrawColor(1);
    u8g2.drawBox(BUGGY_OFFSET_X, BUGGY_OFFSET_Y, WIDTH + BUGGY_OFFSET_X,
                 HEIGHT + BUGGY_OFFSET_Y);
    u8g2.setDrawColor(0);
  } else {
    u8g2.setDrawColor(1);
  }
  u8g2.setFont(u8g2_font_crox4tb_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setFontPosTop();
#if FLIP_SCREEN
  u8g2.setFontDirection(2);
#else
  u8g2.setFontDirection(0);
#endif
}

void graphicsSetup(void) {
  DEBUG_BUFFER[0] = 0;
  u8g2.begin();
  // u8g2.setContrast(100); // NO effect ?
}

void graphicsLoop(void) {
  // invert every 10s
  inverted = ((millis() / 3000) % 2) == 0;
  if (inverted != currently_inverted) {
    currently_inverted = inverted;
  } else if (!dirty) {
    return;
  }

  dirty = false;
  // picture loop
  u8g2_prepare();

#if FLIP_SCREEN
  u8g2.drawStr(WIDTH + BUGGY_OFFSET_X, HEIGHT + BUGGY_OFFSET_Y, DEBUG_BUFFER);
  // u8g2.drawStr(WIDTH + BUGGY_OFFSET_X, (HEIGHT / 2) + BUGGY_OFFSET_Y,
  //              DEBUG_BUFFER2);
#else
  u8g2.drawStr(0, 0, DEBUG_BUFFER);
  // u8g2.drawStr(5, 20, DEBUG_BUFFER2);
#endif
  int icon_width = icon_bluetooth.width;
  // draw icons inverted, first draw a box behind them
  u8g2.setDrawColor(inverted ? 0 : 1);
  u8g2.drawBox(BUGGY_OFFSET_X, BUGGY_OFFSET_Y, BUGGY_OFFSET_X + WIDTH,
               icon_bluetooth.height + 1);
  u8g2.setDrawColor(inverted ? 1 : 0);

  int x_pos = BUGGY_OFFSET_X + icon_bluetooth.width - 4;
  int y_pos = icon_bluetooth.height + BUGGY_OFFSET_Y;

  if (icon_statuses[ICON_BLUETOOTH])
    drawBitmap(x_pos, y_pos, &icon_bluetooth);

  x_pos += icon_width - 2;

  if (icon_statuses[ICON_WIFI])
    drawBitmap(x_pos, y_pos, &icon_wifi);

  x_pos += icon_width - 2;

  if (icon_statuses[ICON_UP])
    drawBitmap(x_pos, y_pos, &icon_up);

  x_pos += icon_width - 2;

  if (icon_statuses[ICON_DOWN])
    drawBitmap(x_pos, y_pos, &icon_down);

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
void setIconStatus(int, bool) {};
#endif
