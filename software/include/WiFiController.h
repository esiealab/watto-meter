#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>

class WiFiController {
public:
    WiFiController(long timezone, byte daysavetime);
    void connect(const char *ssid, const char *password, void (*configModeCallback)(WiFiManager *) = NULL);
    void syncTime();
    struct tm getCurrentTime();
    String formatCurrentTime(struct tm timeinfo, bool millisec = false, bool shortFormat = false);
    String getSSID();
    String getIPAddress();

private:
    WiFiManager wifiManager;
    long timezone;
    byte daysavetime;
};

#endif // WIFI_CONTROLLER_H