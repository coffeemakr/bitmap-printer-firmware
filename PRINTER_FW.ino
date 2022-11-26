#include <HTTPClient.h>

#include <WiFiClientSecure.h>
#include <esp_crt_bundle.h>
#include <ssl_client.h>

#include <HTTPClient.h>

/*
  Please install the following dependent libraries before compiling:
  M5Atom: https://github.com/m5stack/M5Atom
  FastLED: https://github.com/FastLED/FastLED

  How to use:
  1. connect to AP `ATOM_PRINTER-xxxx`
  2. Visit 192.168.4.1 to print
  3. Configure WiFi connection and print data through mqtt server
*/


#include <M5Atom.h>
#include "ATOM_PRINTER.h"
// #include "ATOM_PRINTER_CONFIG.h"
// #include "ATOM_PRINTER_WEB.h"
// #include "ATOM_PRINTER_MQTT.h"
#include "ATOM_PRINTER_WIFI.h"
#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFi.h>
// #include <WiFiSSLClient.h>
#include <ArduinoJson.h>


Preferences preferences;
ATOM_PRINTER printer;

String wifi_ssid;
String wifi_password;


WiFiClientSecure client;
bool wifiConnect(String _wifi_ssid, String _wifi_password, unsigned long timeout) {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(_wifi_ssid.c_str(), _wifi_password.c_str());
  unsigned long start = millis();
  bool is_conneced = false;
  while (millis() - start < timeout) {
    if ((WiFi.status() == WL_CONNECTED)) {
      Serial.println("WiFi Connected!");
      is_conneced = true;
      wifi_ssid = _wifi_ssid;
      wifi_password = _wifi_password;
      // preferences.putString("WIFI_SSID", _wifi_ssid);
      // preferences.putString("WIFI_PWD", _wifi_password);
      // device_state = kWiFiConnected;
      break;
    } else {
      Serial.print(".");
      delay(500);
    }
  }
  return is_conneced;
}

void setup() {
  HTTPClient httpClient;
  M5.begin(true, false, true);
  printer.begin();
  M5.dis.drawpix(0, 0x00ffff);  //初始化状态灯
  preferences.begin("PRINTER_CONFIG");

  preferences.begin("PRINTER_CONFIG");
  wifi_ssid = preferences.getString("wifi_ssid");
  wifi_password = preferences.getString("wifi_pass");
  String bmp_url = preferences.getString("bmp_url");
  String bmp_host = preferences.getString("bmp_host");


  //disableCore0WDT();

  printer.init();
  // printer.newLine(1);
  // Create LED Task

  // mac_addr = WiFi.softAPmacAddress();
  // setWifiMode();
  // setWebServer();
  Serial.println("Connecting to wifi");
  if (wifiConnect(wifi_ssid, wifi_password, 10000)) {
    Serial.println("success");
  } else {
    Serial.println("failed");
  }
  client.setInsecure();

  Serial.println("Connected");
  Serial.print("Connecting to ");
  Serial.println(bmp_host);
  httpClient.begin(client, bmp_host, 443, bmp_url);
  int httpCode = httpClient.GET();
  Serial.print("Status code ");
  Serial.println(httpCode);
  if (httpCode == 200) {
    int fileSize = httpClient.getSize();
    httpClient.setTimeout(0xffff);


    // // get tcp stream
    WiFiClient *stream = httpClient.getStreamPtr();

    // create buffer for read
    uint8_t buff[48 * 100] = { 0 };


    // // read all data from server
    while (httpClient.connected() && (fileSize > 0 || fileSize == -1)) {
      // get available data size
      size_t size = stream->available();
      // Serial.print("available");
      // Serial.println(size);
      if (size >= 48) {
        // read up to 128 byte
        // int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        int c = stream->readBytes(buff, sizeof(buff));
        int ydots = c / 48;
        printer.printBMP(0, 384, ydots, buff);
        Serial.print("printing ");
        Serial.println(ydots);


        if (fileSize > 0) {
          fileSize -= c;
        }
      } else {
        delay(1);
      }
    }

    Serial.println("printed");
  }
  httpClient.end();
}

void loop() {
  // webServer.handleClient();
  M5.update();
}
