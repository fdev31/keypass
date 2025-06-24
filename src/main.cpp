#include "configuration.h"

#include "captive.h"
#include "password.h"
#include <Arduino.h>
#include <esp_sleep.h> // Add this for sleep functions
#include <esp_wifi.h>  // Add this for esp_wifi_set_ps

#ifdef ENABLE_GRAPHICS
#include "graphics.h"
#endif

unsigned long lastClientTime;

int sleeping = 0;
int graphics_initialized = 1;

void setup() {
#ifdef ENABLE_GRAPHICS
  graphicsSetup();
#endif
#if USE_CH9329
  Serial.begin(9600);
#else
  Serial.begin(115200);
#endif
  captiveSetup();
  setUpKeyboard(server);
  lastClientTime = millis();
}

void loop() {
  // START TO SLEEP SLEEP CODE
  int should_sleep = millis() - lastClientTime >=
                     (sleeping ? SLEEP_WAKE_TIME : AUTOSLEEP_TIMEOUT);

  if (!should_sleep) {
    captiveLoop();
#ifdef ENABLE_GRAPHICS
    if (!sleeping) {
      if (!graphics_initialized) {
        graphicsSetup(); // Reinitialize graphics if needed
        graphics_initialized = 1;
      }
      graphicsLoop();
    }
#endif
  }
  delay(30);

  if (!sleeping && should_sleep) {
    Serial.flush(); // Make sure serial output is complete before sleep
#ifdef ENABLE_GRAPHICS
    shutdownGraphics();
#endif
    wifi_ps_type_t ps_type = WIFI_PS_NONE;
    esp_wifi_set_ps(ps_type);
  }

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
  if (should_sleep || sleeping) {
    if (0) {
      // Use timer wakeup to periodically refresh AP state
      esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000);
      esp_light_sleep_start(); // Enter light sleep mode

      // Reset some states
      lastClientTime = millis(); // Reset timer after waking up
      graphics_initialized = 0;
      sleeping = 1;
    }
  }
}
