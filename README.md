A password manager in your keychain

# Requirements

- ESP32 C3 super mini (any controller with Wifi and Serial will do, but an Espressif will require no or little code change)
- (optional) 3D printer
- CH9329 device (better than bitbang and more convenient to plug)

# Installation

- Install platformIO
- Replace the "12345678" WIFI password in `src/configuration.h` with your own
- run "build & upload" in GUI or `pio run --target=upload` in CLI

# Usage

- Plug to the USB port of the device asking for a password
- Connect to "KeyPass" Wifi hotspot
- Follow the instructions to open the interface, or try to connect to any website
- Press on the entry you want to login into, or add a new entry (name & password)
- Several keymaps can be supported (for now French and American keyboard layouts are supported)

# DIY

- Get a microcontroller which has a serial interface and WiFi
- Get some CH9329 device
- Connect the serial of the micro controller to the input of CH9329
- If not using and ESP32 C3, make some minor `platformio.ini` & code changes

# Special thanks

- [Captive portal](https://github.com/CDFER/Captive-Portal-ESP32/) for showing all the tricks for Captive portals and giving a good code base
