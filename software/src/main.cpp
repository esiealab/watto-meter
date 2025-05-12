#include <Arduino.h>

#include "DisplayManager.h"
#include "INAManager.h"
#include "SDManager.h"
#include "WiFiConfig.h"
#include "WiFiController.h"
// clang-format off
#include "APIManager.h"
// clang-format on

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_I2C_ADDR 0x3C

#define SERIAL_SPEED 115200

#define INA_SHUNT_MICRO_OHM 2000                       // 2 mOhm
#define INA_MAXIMUM_AMPS 5                             // Expected max 5A
#define INA_BUS_CONVERSION_TIME 8500                   // Conversion time for bus voltage
#define INA_SHUNT_CONVERSION_TIME 8500                 // Conversion time for shunt voltage
#define INA_AVERAGING_COUNT 128                        // Averaging count
#define INA_MEASUREMENT_MODE INA_MODE_CONTINUOUS_BOTH  // Measurement mode

#define I2C_SDA_PIN 3            // SDA pin
#define I2C_SCL_PIN 10           // SCL pin
#define I2C_CLOCK_SPEED 3400000  // I2C clock speed in Hz

#define TIMEZONE 1
#define DAYLIGHT_SAVING 1

// Frequency of the measure task
const int MEASURE_TASK_PERIOD = 20;
void measureTask(void *parameter);

// Frequency of the display update task (in milliseconds)
const int DISPLAY_TASK_PERIOD = 500;
void updateDisplayTask(void *parameter);

INAManager inaManager;
DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SDManager sdManager;
WiFiController wifiController(TIMEZONE, DAYLIGHT_SAVING);
APIManager apiManager(80);
String fileName = "";
String deviceName = "";
String csvLine = "";

void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("WiFi Config mode");
    Serial.println(WiFi.softAPIP());
    Serial.println(myWiFiManager->getConfigPortalSSID());
    displayManager.showMessage(String(F("Wifi Config Mode\n\nPlease, connect to \n\"")) + myWiFiManager->getConfigPortalSSID() + String(F("\"")));
}

void setup() {
    // Initialize serial communication and I2C
    Serial.begin(SERIAL_SPEED);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_SPEED);
    Serial.println("Watto");

    // Initialize the OLED display
    if (!displayManager.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
        Serial.println(F("OLED display error"));
        while (1);
    }

    // Initialize the INA
    displayManager.showMessage(F("Searching INA..."));
    if (!inaManager.begin(INA_MAXIMUM_AMPS, INA_SHUNT_MICRO_OHM)) {
        Serial.println(F("No INA found, retrying..."));
        while (1);
    }
    inaManager.configure(INA_BUS_CONVERSION_TIME, INA_SHUNT_CONVERSION_TIME, INA_AVERAGING_COUNT, INA_MEASUREMENT_MODE);
    displayManager.showMessage(F("INA found."));

    // Initialize WiFi and sync time
    displayManager.showMessage(F("Connect to WiFi..."));
    wifiController.connect(WIFI_SSID, WIFI_PASSWORD, configModeCallback);
    displayManager.showMessage(F("Sync time..."));
    wifiController.syncTime();

    // Initialize the SD card
    if (!sdManager.begin()) {
        Serial.println("Failed to initialize SD card");
        while (1);
    }

    // Start the web server
    apiManager.begin();

    // Start the measure task
    xTaskCreatePinnedToCore(
        measureTask,     // Task function
        "Measure Task",  // Task name
        4096,            // Stack size
        NULL,            // Task parameter
        3,               // Task priority
        NULL,            // Task handle (not used here)
        1                // Run on core 1
    );

    // Start the display update task
    xTaskCreatePinnedToCore(
        updateDisplayTask,      // Task function
        "Update Display Task",  // Task name
        4096,                   // Stack size
        NULL,                   // Task parameter
        2,                      // Task priority
        NULL,                   // Task handle (not used here)
        1                       // Run on core 1
    );
}

void loop() {
}

void measureTask(void *parameter) {
    TickType_t xlastWakeTime = 0;
    float busVolts, currentMilliAmps, powerWatts;
    struct tm timeinfo;
    while (true) {
        if (apiManager.isMeasuring()) {
            if (fileName == "") {
                struct tm timeinfo = wifiController.getCurrentTime();
                deviceName = apiManager.getDeviceName();
                fileName = "/" + deviceName + "_" + wifiController.formatCurrentTime(timeinfo, false, true) + ".csv";

                // Prepare the CSV file
                sdManager.writeFile(fileName.c_str(), "Timestamp,Address,BusVolts,CurrentMilliAmps,PowerWatts\n");
                Serial.println("Measuring for device: " + deviceName);
            }

            // Retrieve measurements
            busVolts = inaManager.getBusVolts();
            currentMilliAmps = inaManager.getCurrentMilliAmps();
            powerWatts = inaManager.getPowerWatts();
            timeinfo = wifiController.getCurrentTime();

            // Prepare a CSV line
            String csvLine = String(wifiController.formatCurrentTime(timeinfo, true, false) + "," +
                                    String(inaManager.getDeviceAddress(), HEX) + "," +
                                    String(busVolts, 3) + "," +
                                    String(currentMilliAmps, 3) + "," +
                                    String(powerWatts, 3) + "\n");

            // Write to the CSV file
            sdManager.appendFile(fileName.c_str(), csvLine.c_str());
        } else {
            fileName = "";  // Reset the file name if measurements are stopped
        }

        // Delay before the next execution
        vTaskDelayUntil(&xlastWakeTime, pdMS_TO_TICKS(MEASURE_TASK_PERIOD));
    }
}

void updateDisplayTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    String wifiSSID = "";
    String wifiIP = "";
    float busVolts, currentMilliAmps, powerWatts;
    struct tm timeinfo;

    while (true) {
        // Retrieve WiFi information
        wifiSSID = wifiController.getSSID();
        wifiIP = wifiController.getIPAddress();

        // Display WiFi information on the screen
        displayManager.clear();
        displayManager.showWiFiInfo(wifiSSID, wifiIP);

        // Retrieve measurements for display
        busVolts = inaManager.getBusVolts();
        currentMilliAmps = inaManager.getCurrentMilliAmps();
        powerWatts = inaManager.getPowerWatts();
        timeinfo = wifiController.getCurrentTime();

        // Display measurements on the OLED screen
        displayManager.showMeasurements(busVolts, currentMilliAmps, powerWatts,
                                        wifiController.formatCurrentTime(timeinfo, false, false),
                                        inaManager.getDeviceAddress());

        // Delay before the next execution
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DISPLAY_TASK_PERIOD));
    }
}
