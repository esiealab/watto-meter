#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>
#include <time.h>
#include <WiFiManager.h>

class WiFiController {
public:
    WiFiController(const char *ssid, const char *password, long timezone, byte daysavetime);
    void connect();
    void syncTime();
    String getCurrentTime(bool millisec = false);
    String getSSID();
    String getIPAddress();

private:
    const char *ssid;
    const char *password;
    long timezone;
    byte daysavetime;
    WiFiManager wifiManager;
};

#endif // WIFI_CONTROLLER_H