#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "configuration.h"

#if ENABLE_BLUETOOTH

void bluetoothSetup();
void bluetoothLoop();

#endif // ENABLE_BLUETOOTH
#endif // BLUETOOTH_H