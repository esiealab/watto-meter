#ifndef INA_MANAGER_H
#define INA_MANAGER_H

#include <INA.h>

class INAManager {
   public:
    INAManager();
    bool begin(float maxAmps, uint32_t shuntMicroOhm);
    void configure(uint16_t busConversionTime, uint16_t shuntConversionTime, uint16_t averaging, uint8_t mode);
    float getBusVolts();
    float getCurrentMilliAmps();
    float getPowerWatts();
    uint8_t getDeviceAddress();
    const char* getDeviceName();

   private:
    INA_Class ina;
    bool deviceFound;
};

#endif  // INA_MANAGER_H