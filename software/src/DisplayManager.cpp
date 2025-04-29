#include "DisplayManager.h"

DisplayManager::DisplayManager(uint8_t width, uint8_t height, TwoWire* wire, int8_t resetPin)
    : display(width, height, wire, resetPin) {}

bool DisplayManager::begin(uint8_t vccState, uint8_t i2cAddr) {
    return display.begin(vccState, i2cAddr);
}

void DisplayManager::showStartupMessage() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("INA Scan..."));
    display.display();
}

void DisplayManager::showMeasurements(float busVolts, float currentMilliAmps, float powerWatts, const String& currentTime, uint8_t deviceAddress) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);

    // Display current time
    display.println(currentTime);

    // Display INA address
    display.print(F("Addr: 0x"));
    display.println(deviceAddress, HEX);

    // Display voltage, current, and power
    display.setTextSize(2);
    display.print(F("V:"));
    display.print(busVolts, 2);
    display.println(F("V"));
    display.print(F("I:"));
    display.print(currentMilliAmps, 1);
    display.println(F("mA"));
    display.print(F("P:"));
    display.print(powerWatts, 1);
    display.println(F("W"));

    display.display();
}

void DisplayManager::clear() {
    display.clearDisplay();
}