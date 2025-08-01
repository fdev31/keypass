#include "configuration.h"
#if ENABLE_BLUETOOTH
#include "bluetooth.h"
#endif
#if ENABLE_HTTP
#include "captive.h"
#include "http.h"
#endif
#include "bootloader_random.h"
#include "password.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_sleep.h> // Add this for sleep functions
#include <esp_wifi.h>  // Add this for esp_wifi_set_ps

#ifdef ENABLE_GRAPHICS
#include "graphics.h"
#endif
#define SDA_PIN 5
#define SCL_PIN 6

unsigned long lastClientTime;

int sleeping = 0;
int graphics_initialized = 1;

#ifdef ENABLE_GRAPHICS
unsigned long boot_time = 0;
bool version_displayed = false;
#endif

void ping() {
  lastClientTime = millis();
  sleeping = 0;
}

extern void sendHIDInit();

void setup() {
  sendHIDInit();
  bootloader_random_enable();
  srand(millis());
#ifdef ENABLE_GRAPHICS
  graphicsSetup();
  char version_string[50];
  snprintf(version_string, sizeof(version_string), "%d:%s", HW_TYPE, VERSION);
  printText(1, version_string);
  boot_time = millis();
  version_displayed = true;
#endif
#if USE_CH9329
  Serial.begin(9600);
#else
  Serial.begin(115200);
#endif
#if ENABLE_HTTP
  captiveSetup();
  setupHttp(server);
#endif
#if ENABLE_BLUETOOTH
  bluetoothSetup();
#endif
  lastClientTime = millis();
  Wire.begin(SDA_PIN, SCL_PIN);
}

void loop() {
// START TO SLEEP SLEEP CODE
// int should_sleep = millis() - lastClientTime >=
//                    (sleeping ? SLEEP_WAKE_TIME : AUTOSLEEP_TIMEOUT);

// if (!should_sleep) {
#if ENABLE_HTTP
  captiveLoop();
  yield();
#endif
#if ENABLE_BLUETOOTH
  bluetoothLoop(); // Handle Bluetooth commands if needed
  yield();
#endif
#ifdef ENABLE_GRAPHICS
  if (version_displayed && (millis() - boot_time > 2000)) {
    printText(1, "Welcome!");
    version_displayed = false;
  }
  // if (!sleeping) {
  if (!graphics_initialized) {
    graphicsSetup(); // Reinitialize graphics if needed
    graphics_initialized = 1;
    yield();
  }
  graphicsLoop();
  yield();
  // }
#endif
  // }

  // if (!sleeping && should_sleep) {
  //   Serial.flush(); // Make sure serial output is complete before sleep
  //   yield();
  // #ifdef ENABLE_GRAPHICS
  //   shutdownGraphics();
  //   yield();
  // #endif
  //   wifi_ps_type_t ps_type = WIFI_PS_NONE;
  //   esp_wifi_set_ps(ps_type);
  //   yield();
  // }

#if 0
    // Set AP beacon interval to ensure visibility
    wifi_ap_config_t ap_config = {0};
    wifi_config_t wifi_config = {0};
    esp_wifi_get_config(WIFI_IF_AP, &wifi_config);
    ap_config = wifi_config.ap;
    ap_config.beacon_interval =
        100; // Shorter beacon interval (default is 100ms)
    wifi_config.ap = ap_config;
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
#endif
  // if (should_sleep || sleeping) {
  // #if !DEBUG
  //   // Use timer wakeup to periodically refresh AP state
  //   esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000);
  //   delay(1);
  //   yield();
  //   esp_light_sleep_start(); // Enter light sleep mode

  //   // Reset some states
  //   lastClientTime = millis(); // Reset timer after waking up
  //   graphics_initialized = 0;
  //   sleeping = 1;
  // }
  // #endif
}
