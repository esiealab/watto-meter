#include "INAManager.h"

INAManager::INAManager() : deviceFound(false) {}

bool INAManager::begin(float maxAmps, uint32_t shuntMicroOhm) {
    uint8_t devices = ina.begin(maxAmps, shuntMicroOhm);
    deviceFound = (devices > 0);  // Vérifie si au moins un périphérique a été détecté
    return deviceFound;
}

void INAManager::configure(uint16_t busConversionTime, uint16_t shuntConversionTime, uint16_t averaging, uint8_t mode) {
    if (deviceFound) {
        ina.setBusConversion(busConversionTime);      // Conversion time for bus voltage
        ina.setShuntConversion(shuntConversionTime);  // Conversion time for shunt voltage
        ina.setAveraging(averaging);                  // Averaging count
        ina.setMode(mode);                            // Measurement mode
    }
}

float INAManager::getBusVolts() {
    return deviceFound ? ina.getBusMilliVolts() / 1000.0 : 0.0;
}

float INAManager::getCurrentMilliAmps() {
    return deviceFound ? ina.getBusMicroAmps() / 1000.0 : 0.0;
}

float INAManager::getPowerWatts() {
    return deviceFound ? ina.getBusMicroWatts() / 1000000.0 : 0.0;
}

uint8_t INAManager::getDeviceAddress() {
    return deviceFound ? ina.getDeviceAddress() : 0;
}

const char *INAManager::getDeviceName() {
    return deviceFound ? ina.getDeviceName() : "No Device";
}