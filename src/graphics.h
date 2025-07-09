#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include <stdint.h>

void printText(uint8_t, const char *);
void graphicsSetup();
void graphicsLoop();
void shutdownGraphics();
#endif
