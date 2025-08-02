#include "configuration.h"
#if ENABLE_BLUETOOTH
#include "bluetooth.h"
#endif
#if ENABLE_HTTP
#include "captive.h"
#include "http.h"
#endif
#include "WiFi.h"
#include "bootloader_random.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "esp_bt.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
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
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
  sendHIDInit();
  bootloader_random_enable();
  srand(millis());
#ifdef ENABLE_GRAPHICS
  graphicsSetup();
  char version_string[50];
  snprintf(version_string, sizeof(version_string), "%d@%s", HW_TYPE, VERSION);
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
  setIconStatus(ICON_WIFI, true);
#endif
#if ENABLE_BLUETOOTH
  bluetoothSetup();
  setIconStatus(ICON_BLUETOOTH, true);
#endif
  lastClientTime = millis();
  Wire.begin(SDA_PIN, SCL_PIN);
}
void loop() {
  int should_sleep = millis() - lastClientTime >=
                     (sleeping ? SLEEP_WAKE_TIME : AUTOSLEEP_TIMEOUT);

  if (!should_sleep) {
#if ENABLE_HTTP
    captiveLoop();
    yield();
#endif
#if ENABLE_BLUETOOTH
    bluetoothLoop();
    yield();
#endif
#ifdef ENABLE_GRAPHICS
    if (version_displayed && (millis() - boot_time > 2000)) {
      printText(1, "  _(^-^)_");
      version_displayed = false;
    }
    if (!sleeping) {
      if (!graphics_initialized) {
        graphicsSetup();
        graphics_initialized = 1;
        yield();
      }
      graphicsLoop();
      yield();
    }
#endif
  }
  if (!sleeping && should_sleep) {

    // First put WiFi in power save mode to gracefully reduce activity
    wifi_ps_type_t ps_type = WIFI_PS_MAX_MODEM;
    esp_wifi_set_ps(ps_type);

    yield();
    delay(100); // Give WiFi time to enter power save

#ifdef ENABLE_GRAPHICS
    shutdownGraphics();
#endif
    yield();
  }

#if !DEBUG
  if (should_sleep || sleeping) {
    esp_deep_sleep_start();
  }
#endif

  // Wake-up: reset states
  if (should_sleep || sleeping) {
    lastClientTime = millis();
    graphics_initialized = 0;
    sleeping = 1;
  }
}
