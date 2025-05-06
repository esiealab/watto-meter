#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>

class WiFiController {
   public:
    WiFiController(const char *ssid, const char *password, long timezone, byte daysavetime);
    void connect(void (*configModeCallback)(WiFiManager *) = NULL);
    void syncTime();
    struct tm getCurrentTime();
    String formatCurrentTime(struct tm timeinfo, bool millisec = false, bool shortFormat = false);
    String getSSID();
    String getIPAddress();

   private:
    const char *ssid;
    const char *password;
    long timezone;
    byte daysavetime;
    WiFiManager wifiManager;
};

#endif  // WIFI_CONTROLLER_H