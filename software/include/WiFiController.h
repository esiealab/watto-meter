#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>

class WiFiController {
public:
    WiFiController(long timezone, byte daysavetime);
    void connect(void (*configModeCallback)(WiFiManager *) = NULL);
    void syncTime();
    struct tm getCurrentTime();
    String formatCurrentTime(struct tm timeinfo, bool millisec = false, bool shortFormat = false);
    String getSSID();
    String getIPAddress();
    String getHostname();
    bool isConnected();
    bool isConfigMode();
    String getInfosMessage();

private:
    WiFiManager wifiManager;
    long timezone;
    byte daysavetime;
    String uniqueHostname;
};

#endif // WIFI_CONTROLLER_H