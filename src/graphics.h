#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include "icons.h"
#include <stdint.h>

#define ICON_BLUETOOTH 0
#define ICON_WIFI 1
#define ICON_UP 2
#define ICON_DOWN 3

void printText(uint8_t, const char *);
void graphicsSetup();
void graphicsLoop();
void shutdownGraphics();
void setIconStatus(int index, bool enabled);
#endif
