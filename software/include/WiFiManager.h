#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <time.h>

class WiFiManager {
public:
    WiFiManager(const char *ssid, const char *password, long timezone, byte daysavetime);
    void connect();
    void syncTime();
    String getCurrentTime(bool millisec = false);

private:
    const char *ssid;
    const char *password;
    long timezone;
    byte daysavetime;
};

#endif // WIFI_MANAGER_H