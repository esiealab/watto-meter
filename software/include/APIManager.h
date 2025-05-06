#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "SD.h"
#include "SPIFFS.h"

class APIManager {
   public:
    APIManager(int port, String mainFolder = "/");
    void begin();
    bool isMeasuring();
    String getDeviceName();

   private:
    AsyncWebServer server;
    bool startMeasures;
    String device;
    String mainFolder;

    void handleRoot(AsyncWebServerRequest *request);
    void handleStartMeasures(AsyncWebServerRequest *request);
    void handleStopMeasures(AsyncWebServerRequest *request);
    void handleSetDevice(AsyncWebServerRequest *request);
    void handleListFiles(AsyncWebServerRequest *request);
    void handleDownloadFile(AsyncWebServerRequest *request);
    void handleDeleteFile(AsyncWebServerRequest *request);
};

#endif  // API_MANAGER_H