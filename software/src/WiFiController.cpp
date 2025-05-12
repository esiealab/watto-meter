#include "WiFiController.h"

WiFiController::WiFiController(long timezone, byte daysavetime)
    : timezone(timezone), daysavetime(daysavetime) {}

void WiFiController::connect(const char *ssid, const char *password, void (*configModeCallback)(WiFiManager *)) {
    if (configModeCallback != NULL) {
        wifiManager.setAPCallback(configModeCallback);
    } else {
        wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
            Serial.println("Entered config mode");
            Serial.println(WiFi.softAPIP());
            Serial.println(myWiFiManager->getConfigPortalSSID());
        });
    }
    wifiManager.autoConnect(ssid, password);
    Serial.printf("WiFi connected to %s\n", ssid);
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
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

struct tm WiFiController::getCurrentTime() {
    struct tm timeinfo = {};
    if (!getLocalTime(&timeinfo))
        return timeinfo;
    return timeinfo;
}

String WiFiController::formatCurrentTime(struct tm timeinfo, bool millisec, bool shortFormat) {
    char buffer[24];
    int milliseconds = millis() % 1000;  // Get milliseconds
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