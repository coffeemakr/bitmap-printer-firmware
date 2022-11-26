#ifndef _ATOM_PRINTER_WIFI_H
#define _ATOM_PRINTER_WIFI_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <Preferences.h>
#include "ATOM_PRINTER_CONFIG.h"


bool wifiConnect(String ssid, String password, unsigned long timeout);
void setWifiMode();

#endif
