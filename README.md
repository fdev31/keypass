A password manager in your keychain

# Requirements

- ESP32 S2 mini
- (optional) 3D printer
- CH9329 device or USB-C cable

# Installation

- Install platformIO
- Edit `src/passwords.h` and `src/main.cpp` (mainly for the WIFI password)
- run "build & upload" in GUI or `pio run --target=upload` in CLI

# Usage

- Plug or press *RESET*
- Click *BOOT* a number of times (once == first password, twice == second, ...)
- Connect to the WIFI with your phone
- Enjoy!
