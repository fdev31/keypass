/**
 * NOTE: https://usb.org/sites/default/files/hut1_3_0.pdf Page 91 !
 */
#include "hidkeyboard.h"
#include "keymap.h"
#include <Arduino.h>
#include <WiFi.h>

#define USE_WIFI 0
#define WARMUP_TIME 1500
#define KEYPRESS_INTERVAL 60
#define BUTTON 0 // BOOT BUTTON

// Wifi stuff {{{
bool devicePaired = false;
#define SSID "KeyPass"
// ALL THE SECURITY IS IN THIS PASSWORD - use something
// longer & more complex !!
#define PASSWORD "AAAAAAAAAAAAAAAA"

#define MAX_CONNECT_RETRY 60

WiFiServer wifiServer(80);
WiFiClient wifiClient;

void setupWiFi() {
  Serial.println("Configuring access point...");
  WiFi.softAP(SSID, PASSWORD);

  IPAddress ipAddress = WiFi.softAPIP();
  Serial.print("Access Point IP address: ");
  Serial.println(ipAddress);

  wifiServer.begin();
}

void handleWiFi() {
  wifiClient = wifiServer.available();
  if (wifiClient) {
    devicePaired = true;
  }
}
// }}}
typedef struct {
  unsigned char flags;
  const char *password;
} Password;

enum { FLAG_NONE, FLAG_FKEY = 1 };
// Include or declare list of passwords, first parameter is the flag, eg:
//
// Password passwords[] = {
//     {FLAG_NONE, "qwerty1234"},
//     {FLAG_FKEY, "12345"},
// };
// #define NB_PASSWORDS 2

#include "passwords.h"
#if CFG_TUD_HID
//
HIDkeyboard dev;
int has_run = 0;
int connect_check_count = 0;

double startup_time = 0;

// Send some Function key
void sendFKey(char c) { dev.sendKey(57 + (c == '0' ? 10 : c - '0')); }
// Send a normal character
void sendKey(char c) {
  dev.sendKey(KBD_MAP[c][0], KBD_MAP[c][1]);
}
// Sends a string,
// optionally using 0-9 digits only and sending them via F keys
void sendCode(const char *text, uint8_t flags = 0) {
  bool fkeys = (flags & FLAG_FKEY) == FLAG_FKEY;
  // Pointer to sendFKey or sendKey:
  void (*sendPtr)(char) = fkeys ? sendFKey : sendKey;

  while (*text) {
    sendPtr(*text);
    delay(KEYPRESS_INTERVAL);
    text++;
  }
}

void start_warmup_timer() { startup_time = millis() + WARMUP_TIME; }

void setup() {
  // Wifi setup
#if USE_WIFI
  setupWiFi();
#endif
  // USB HID setup
  dev.setBaseEP(2);
  dev.manufacturer("Fab Corp.");
  dev.product("Blue Square Thing");
  dev.deviceID(0xfab, 0xface);
  dev.serial("4321-567890");
  dev.begin();
  // Button setup
  pinMode(0, INPUT_PULLUP);
  // state machine setup
  start_warmup_timer();
}

static int click_count = 0;
static int btn_state = HIGH;

void loop() {
  if (millis() < startup_time) {
    int but_state = digitalRead(BUTTON);
    if (but_state == LOW && but_state != btn_state) {
      click_count++;
      start_warmup_timer();
    }
    btn_state = but_state;
    delay(50);
    return;
  }
  if (has_run) {
    // dev.sendString("sleep\n");
    delay(1000);
    // STOP
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
  }

  if (click_count) {
    if (click_count > NB_PASSWORDS) {
      has_run = 1;
      sendCode("KO");
      return;
    }
    // Check pairing state: refresh it or type password
    if (devicePaired) {
      has_run = 1;
      sendCode(passwords[click_count - 1].password,
               passwords[click_count - 1].flags);
      dev.sendKey(40);
    } else {
      has_run = connect_check_count++ > MAX_CONNECT_RETRY;
      if (!has_run) {
#if USE_WIFI
        handleWiFi();
#else
        devicePaired = true;
#endif
      }
      delay(500);
    }
  }
}

#endif
