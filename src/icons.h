// Generated bitmap header
#ifndef ICONS_H
#define ICONS_H
#include <stdint.h>

typedef struct {
    const uint8_t *data;
    int width;
    int height;
} Bitmap;

// Bitmap for icon_bluetooth.png (20x20)
extern const uint8_t icon_bluetooth_data[];
extern const Bitmap icon_bluetooth;

// Bitmap for icon_wifi.png (20x20)
extern const uint8_t icon_wifi_data[];
extern const Bitmap icon_wifi;

// Bitmap for icon_up.png (20x20)
extern const uint8_t icon_up_data[];
extern const Bitmap icon_up;

// Bitmap for icon_down.png (20x20)
extern const uint8_t icon_down_data[];
extern const Bitmap icon_down;

// Collection of all bitmap icons
extern const Bitmap* icons[];
const int icons_count = 4;

#endif // ICONS_H