#include <Arduino.h> //not needed in the arduino ide
#include <EEPROM.h>
#include <string.h>

// Captive Portal
#include <AsyncTCP.h> //https://github.com/me-no-dev/AsyncTCP using the latest dev version from @me-no-dev
#include <DNSServer.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include <esp_wifi.h>          //Used for mpdu_rx_disable android workaround

// Pre reading on the fundamentals of captive portals
// https://textslashplain.com/2022/06/24/captive-portals/

const char *ssid =
    "KeyPass"; // FYI The SSID can't have a space in it.
               // const char * password = "12345678"; //Atleast 8 chars
const char *password = NULL; // no password
                             //

#define MAX_CLIENTS 4
#define WIFI_CHANNEL 6

const IPAddress localIP(4, 3, 2, 1); // the IP address the web server, Samsung
                                     // requires the IP to be in public space
const IPAddress gatewayIP(4, 3, 2,
                          1); // IP address of the network should be the same
                              // as the local IP for captive portals
const IPAddress subnetMask(255, 255, 255, 0);

const String localIPURL = "http://4.3.2.1";

DNSServer dnsServer;
AsyncWebServer server(80);

void setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP) {

  // Set the TTL for DNS response and start the DNS server
  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", localIP);
}

void startSoftAccessPoint(const char *ssid, const char *password,
                          const IPAddress &localIP,
                          const IPAddress &gatewayIP) {
  // Define the maximum number of clients that can connect to the server
#define MAX_CLIENTS 4
  // Define the WiFi channel to be used (channel 6 in this case)
#define WIFI_CHANNEL 6

  // Set the WiFi mode to access point and station
  WiFi.mode(WIFI_MODE_AP);

  // Define the subnet mask for the WiFi network
  const IPAddress subnetMask(255, 255, 255, 0);

  // Configure the soft access point with a specific IP and subnet mask
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);

  // Start the soft access point with the given ssid, password, channel, max
  // number of clients
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);

  // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
  esp_wifi_stop();
  esp_wifi_deinit();
  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
  my_config.ampdu_rx_enable = false;
  esp_wifi_init(&my_config);
  esp_wifi_start();
  vTaskDelay(100 / portTICK_PERIOD_MS); // Add a small delay
}

void setUpWebserver(AsyncWebServer &server, const IPAddress &localIP) {
  //======================== Webserver ========================
  // WARNING IOS (and maybe macos) WILL NOT POP UP IF IT CONTAINS THE WORD
  // "Success" https://www.esp8266.com/viewtopic.php?f=34&t=4398 SAFARI (IOS) IS
  // STUPID, G-ZIPPED FILES CAN'T END IN .GZ
  // https://github.com/homieiot/homie-esp8266/issues/476 this is fixed by the
  // webserver serve static function. SAFARI (IOS) there is a 128KB limit to the
  // size of the HTML. The HTML can reference external resources/images that
  // bring the total over 128KB SAFARI (IOS) popup browser has some severe
  // limitations (javascript disabled, cookies disabled)

  // Required
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
    request->redirect("http://logout.net");
  }); // windows 11 captive portal workaround
  server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
    request->send(404);
  }); // Honestly don't understand what this is but a 404 stops win 10 keep
      // calling this repeatedly and panicking the esp32 :)

  // Background responses: Probably not all are Required, but some are. Others
  // might speed things up? A Tier (commonly used by modern systems)
  server.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  }); // android captive portal redirect
  server.on("/redirect", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  }); // microsoft redirect
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  }); // apple call home
  server.on("/canonical.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  }); // firefox captive portal call home
  server.on("/success.txt", [](AsyncWebServerRequest *request) {
    request->send(200);
  }); // firefox captive portal call home
  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  }); // windows call home

  // B Tier (uncommon)
  //  server.on("/chrome-variations/seed",[](AsyncWebServerRequest
  //  *request){request->send(200);}); //chrome captive portal call home
  //  server.on("/service/update2/json",[](AsyncWebServerRequest
  //  *request){request->send(200);}); //firefox?
  //  server.on("/chat",[](AsyncWebServerRequest
  //  *request){request->send(404);}); //No stop asking Whatsapp, there is no
  //  internet connection server.on("/startpage",[](AsyncWebServerRequest
  //  *request){request->redirect(localIPURL);});

  // return 404 to webpage icon
  server.on("/favicon.ico", [](AsyncWebServerRequest *request) {
    request->send(404);
  }); // webpage icon

  // the catch all
  server.onNotFound(
      [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
};

void captiveSetup() {
  startSoftAccessPoint(ssid, password, localIP, gatewayIP);
  setUpDNSServer(dnsServer, localIP);
  setUpWebserver(server, localIP);
  server.begin();
}
void captiveLoop() {
  dnsServer.processNextRequest(); // I call this atleast every 10ms in my other
}
