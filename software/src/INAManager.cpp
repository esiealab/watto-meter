#include "INAManager.h"

INAManager::INAManager() : devicesFound(0) {}

uint8_t INAManager::begin(float maxAmps, uint32_t shuntMicroOhm) {
    devicesFound = ina.begin(maxAmps, shuntMicroOhm);
    return devicesFound;
}

void INAManager::configure(uint16_t busConversionTime, uint16_t shuntConversionTime, uint16_t averaging, uint8_t mode) {
    ina.setBusConversion(busConversionTime);   // Conversion time for bus voltage
    ina.setShuntConversion(shuntConversionTime); // Conversion time for shunt voltage
    ina.setAveraging(averaging);               // Averaging count
    ina.setMode(mode);                         // Measurement mode
}

uint8_t INAManager::getDevicesFound() const {
    return devicesFound;
}

float INAManager::getBusVolts(uint8_t deviceIndex) {
    return ina.getBusMilliVolts(deviceIndex) / 1000.0;
}

float INAManager::getCurrentMilliAmps(uint8_t deviceIndex) {
    return ina.getBusMicroAmps(deviceIndex) / 1000.0;
}

float INAManager::getPowerWatts(uint8_t deviceIndex) {
    return ina.getBusMicroWatts(deviceIndex) / 1000000.0;
}

uint8_t INAManager::getDeviceAddress(uint8_t deviceIndex) {
    return ina.getDeviceAddress(deviceIndex);
}

const char* INAManager::getDeviceName(uint8_t deviceIndex) {
    return ina.getDeviceName(deviceIndex);
}