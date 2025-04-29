#include "WiFiManager.h"

WiFiManager::WiFiManager(const char *ssid, const char *password, long timezone, byte daysavetime)
    : ssid(ssid), password(password), timezone(timezone), daysavetime(daysavetime) {}

void WiFiManager::connect() {
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
}

void WiFiManager::syncTime() {
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

String WiFiManager::getCurrentTime(bool millisec) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "Failed to get time";
    char buffer[24];
    int milliseconds = millis() % 1000; // Get milliseconds
    if(millisec == true) {
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