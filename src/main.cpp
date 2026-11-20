#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
// #include <WiFi.h>
#include <global.h>

// put function declarations here:
int myFunction(int, int);
const char* ssid     = "diego2";
const char* password = "diegorita"; 
bool status_print_enable = true;

// AsyncWebServer server(80);

unsigned long ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
    // ethernet_initialization();
  server_initialization();
  // TCP_initialization();
  // UDP_server_activation();

}

void loop() {
  OTA_handler();
  // TCP_handler();
  // UDP_handler()
  // telemetry();
  status_print();

}