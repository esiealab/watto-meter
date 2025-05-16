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
    uint64_t getCurrentTime_us();
    String formatCurrentTime(uint64_t time_us, bool millisec = false, bool shortFormat = false);
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

#endif  // WIFI_CONTROLLER_H