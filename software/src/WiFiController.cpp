#include "WiFiController.h"

WiFiController::WiFiController(long timezone, byte daysavetime)
    : timezone(timezone), daysavetime(daysavetime) {
    uniqueHostname = "WM-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    uniqueHostname.toUpperCase();
    wifiManager.setHostname(uniqueHostname.c_str());

    wifiManager.setConfigPortalBlocking(true);
    // wifiManager.setConfigPortalTimeout(30);  // Set timeout to 30 seconds
}

void WiFiController::connect(void (*configModeCallback)(WiFiManager *)) {
    if (configModeCallback != NULL) {
        wifiManager.setAPCallback(configModeCallback);
    } else {
        wifiManager.setAPCallback([this](WiFiManager *myWiFiManager) {
            Serial.println("WiFi Config mode");
            Serial.println(WiFi.softAPIP());
            Serial.println(myWiFiManager->getConfigPortalSSID());
            Serial.println("Please connect to the WiFi network and open the config portal.");
        });
    }

    wifiManager.autoConnect(uniqueHostname.c_str(), NULL);
    Serial.printf("WiFi connected to %s\n", getSSID());
    Serial.printf("IP address: %s\n", getIPAddress());
    Serial.printf("Hostname: %s\n", getHostname());
}

void WiFiController::syncTime() {
    configTime(3600 * timezone, daysavetime * 3600, "pool.ntp.org", "time.nist.gov");
    struct tm timeinfo;    
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.printf("Time synchronized: %d-%02d-%02d %02d:%02d:%02d\n",
                  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

uint64_t WiFiController::getCurrentTime_us() {
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t time_us = (uint64_t)tv_now.tv_sec * 1000000L + (uint64_t)tv_now.tv_usec;
    return time_us;
}

String WiFiController::formatCurrentTime(uint64_t time_us, bool millisec, bool shortFormat) {
    char buffer[24];
    struct tm timeinfo;
    uint16_t milliseconds = 0;

    // Convert microseconds to seconds and get the timeinfo
    time_t seconds = time_us / 1000000;
    localtime_r(&seconds, &timeinfo);
    if (millisec) {
        milliseconds = time_us % 1000000 / 1000;
    }

    if (shortFormat) {
        sprintf(buffer, "%d-%02d-%02d-%02d%02d%02d",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else if (millisec) {
        sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d.%03d",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, milliseconds);
    } else {
        sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
    return String(buffer);
}

String WiFiController::getSSID() {
    return WiFi.SSID();
}

String WiFiController::getIPAddress() {
    return WiFi.localIP().toString();
}

String WiFiController::getHostname() {
    return WiFi.getHostname();
}

bool WiFiController::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiController::isConfigMode() {
    return wifiManager.getConfigPortalActive();
}

String WiFiController::getInfosMessage() {
    String info;
    if (!isConfigMode()) {
        info = getSSID() + "\n";
        info += getIPAddress() + "\n";
        info += getHostname() + "\n";
    } else {
        info = WiFi.softAPSSID() + " (AP)\n";
        info += WiFi.softAPIP().toString() + "\n";
        info += "Please configure.\n";
    }
    return info;
}