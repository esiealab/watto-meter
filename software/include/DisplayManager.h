#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_SSD1306.h>

const unsigned char wifiicon[] PROGMEM = {  // wifi icon
    0x00, 0xff, 0x00, 0x7e, 0x00, 0x18, 0x00, 0x00};
class DisplayManager {
   public:
    DisplayManager(uint8_t width, uint8_t height, TwoWire *wire, int8_t resetPin);
    bool begin(uint8_t vccState, uint8_t i2cAddr);
    void showMessage(String message);
    void showMeasurements(float busVolts, float currentMilliAmps, float powerWatts, const String &currentTime, uint8_t deviceAddress);
    void showWiFiInfo(const String &ssid, const String &ip);
    void clear();

   private:
    Adafruit_SSD1306 display;
};

#endif  // DISPLAY_MANAGER_H