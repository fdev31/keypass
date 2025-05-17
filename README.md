A password manager in your keychain

# Requirements

- ESP32 C3 super mini (can easily support more)
- (optional) 3D printer
- CH9329 device (bitbang USB disabled since this is more reliable, but may come back)

# Installation

- Install platformIO
- Replace the "12345678" WIFI password in `src/captive.cpp` with your own
- run "build & upload" in GUI or `pio run --target=upload` in CLI

# Usage

- Plug to the USB port of the device asking for a password
- Connect to "KeyPass" Wifi hotspot
- Follow the instructions to open the interface, or try to connect to any website
- Press on the entry you want to login into, or add a new entry (name & password)
- Several keymaps can be supported (for now French and American keyboard layouts are supported)

> [!warning]
> Default wifi password is "12345678", change it in `src/captive.cpp`!

# DIY

- Get a microcontroller which has a serial interface and WiFi
- Get some CH9329 device
- Connect the serial of the micro controller to the input of CH9329
- If not using and ESP32 C3, make some minor `platformio.ini` & code changes
