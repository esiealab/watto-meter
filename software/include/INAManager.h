#ifndef INA_MANAGER_H
#define INA_MANAGER_H

#include <INA.h>

class INAManager {
public:
    INAManager();
    uint8_t begin(float maxAmps, uint32_t shuntMicroOhm);
    void configure();
    void configure(uint16_t busConversionTime, uint16_t shuntConversionTime, uint16_t averaging, uint8_t mode);
    uint8_t getDevicesFound() const;
    float getBusVolts(uint8_t deviceIndex);
    float getCurrentMilliAmps(uint8_t deviceIndex);
    float getPowerWatts(uint8_t deviceIndex);
    uint8_t getDeviceAddress(uint8_t deviceIndex);
    const char* getDeviceName(uint8_t deviceIndex);

private:
    INA_Class ina;
    uint8_t devicesFound;
};

#endif // INA_MANAGER_H