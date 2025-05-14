#include "DisplayManager.h"

DisplayManager::DisplayManager(uint8_t width, uint8_t height, TwoWire *wire, int8_t resetPin)
    : display(width, height, wire, resetPin) {}

bool DisplayManager::begin(uint8_t vccState, uint8_t i2cAddr) {
    bool test = display.begin(vccState, i2cAddr);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    return test;
}

void DisplayManager::showMessage(String message, bool clearScreen, uint8_t startLine, bool forceDisplay, const unsigned char *icon) {
    int16_t startColumn = 0;
    if (clearScreen) {
        display.clearDisplay();
    }
    if (icon != NULL) {
        display.drawBitmap(startColumn, startLine * 10, icon, 8, 8, WHITE);
        startColumn = 10;
    }
    display.setCursor(startColumn, startLine * 10);  // Assuming each line is 10 pixels high
    display.println(message);
    if (forceDisplay) {
        display.display();
    }
}

void DisplayManager::clear() {
    display.clearDisplay();
}
