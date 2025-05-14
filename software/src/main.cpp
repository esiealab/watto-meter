#include <Arduino.h>

#include "DisplayManager.h"
#include "INAManager.h"
#include "SD.h"
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

#define QUEUE_SIZE 1000

// Frequency of the measure task
const int MEASURE_TASK_PERIOD = 2;
const uint8_t NB_MEASURE_BEFORE_SD = 50;  // Number of measures before writing to SD
void readI2CTask(void *parameter);
void writeSDTask(void *parameter);

// Frequency of the display update task (in milliseconds)
const int DISPLAY_TASK_PERIOD = 1000;
void updateDisplayTask(void *parameter);

void setupWiFiServer(void *parameter);

struct MeasurementData {
    float busVolts;
    float currentMilliAmps;
    struct tm timeinfo;
} measurement;

INAManager inaManager;
DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiController wifiController(TIMEZONE, DAYLIGHT_SAVING);
APIManager apiManager(80);
String fileName = "";
String deviceName = "";
String csvLine = "";

QueueHandle_t measurementQueue;

void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("WiFi Config mode");
    Serial.println(WiFi.softAPIP());
    Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
    // Initialize serial communication and I2C
    Serial.begin(SERIAL_SPEED);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_SPEED);

    // Initialize the SD card
    if (!SD.begin()) {
        Serial.println(F("SD card error"));
        while (1);
    }

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

    // Create the queue for measurements
    measurementQueue = xQueueCreate(QUEUE_SIZE, sizeof(struct MeasurementData));
    if (measurementQueue == NULL) {
        Serial.println("Failed to create measurement queue");
        while (1);
    }

    // Start the I2C reading task
    xTaskCreatePinnedToCore(
        readI2CTask,      // Task function
        "Read I2C Task",  // Task name
        4096,             // Stack size
        NULL,             // Task parameter
        4,                // Task priority (higher priority)
        NULL,             // Task handle (not used here)
        1                 // Run on core 1
    );

    // Start the SD card writing task
    xTaskCreatePinnedToCore(
        writeSDTask,      // Task function
        "Write SD Task",  // Task name
        4096,             // Stack size
        NULL,             // Task parameter
        3,                // Task priority (lower priority)
        NULL,             // Task handle (not used here)
        1                 // Run on core 1
    );

    // Create a task for initialization
    xTaskCreatePinnedToCore(
        setupWiFiServer,  // Task function
        "Init Task",      // Task name
        4096,             // Stack size
        NULL,             // Task parameter
        1,                // Task priority
        NULL,             // Task handle (not used here)
        1                 // Run on core 1
    );

    // Start the display update task
    xTaskCreatePinnedToCore(
        updateDisplayTask,      // Task function
        "Update Display Task",  // Task name
        4096,                   // Stack size
        NULL,                   // Task parameter
        3,                      // Task priority
        NULL,                   // Task handle (not used here)
        1                       // Run on core 1
    );
}

void loop() {
}

void setupWiFiServer(void *parameter) {
    // Initialize WiFi and sync time
    displayManager.showMessage(F("Connect to WiFi..."));
    wifiController.connect(configModeCallback);
    displayManager.showMessage(F("Sync time..."));
    wifiController.syncTime();
    // Start the web server
    apiManager.begin();

    // Delete the task after initialization is complete
    vTaskDelete(NULL);
}

void updateDisplayTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    float busVolts, currentMilliAmps, powerWatts;
    struct tm timeinfo;

    while (true) {
        displayManager.showMessage(wifiController.getInfosMessage(), true, 0, false, wifiicon);

        // Retrieve measurements for display
        timeinfo = wifiController.getCurrentTime();
        busVolts = inaManager.getBusVolts();
        currentMilliAmps = inaManager.getCurrentMilliAmps();

        UBaseType_t queueFillPercentage = (uxQueueMessagesWaiting(measurementQueue) * 100) / (uxQueueSpacesAvailable(measurementQueue) + uxQueueMessagesWaiting(measurementQueue));
        displayManager.showMessage(String(wifiController.formatCurrentTime(timeinfo, false, false) + "\nQueue: " + String(queueFillPercentage) + "%\n" +
                                          "Volt: " + String(busVolts, 2) + "V\n" +
                                          "Current: " + String(currentMilliAmps, 2) + "mA\n"),
                                   false, 3, true, NULL);

        // Delay before the next execution
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DISPLAY_TASK_PERIOD));
    }
}

void readI2CTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true) {
        if (apiManager.isMeasuring()) {
            // Read data from INA
            measurement.timeinfo = wifiController.getCurrentTime();
            measurement.busVolts = inaManager.getBusVolts();
            measurement.currentMilliAmps = inaManager.getCurrentMilliAmps();

            // Send data to the queue
            if (xQueueSend(measurementQueue, &measurement, 0) != pdPASS) {
                Serial.println("Measurement queue full, data dropped");
                vTaskDelay(pdMS_TO_TICKS(100));  // Wait before retrying
            }
        }

        // Delay before the next reading
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MEASURE_TASK_PERIOD));
    }
}

void writeSDTask(void *parameter) {
    File file;
    String csvLine;
    uint8_t nbLines = 0;

    while (true) {
        if (!apiManager.isMeasuring()) {
            fileName = "";  // Reset the file name if measurements are stopped
        } else if (fileName == "") {
            // Create a new file if necessary
            fileName = "/" + apiManager.getDeviceName() + "_" +
                       wifiController.formatCurrentTime(measurement.timeinfo, false, true) + ".csv";
            file = SD.open(fileName.c_str(), FILE_WRITE);
            file.println("Timestamp,BusVolts,CurrentMilliAmps");
            file.close();

            Serial.println("Measuring for device: " + apiManager.getDeviceName());
        }
        // Wait for data from the queue
        csvLine = "";
        nbLines = 0;
        while (xQueueReceive(measurementQueue, &measurement, portMAX_DELAY) == pdPASS && fileName != "" && nbLines < NB_MEASURE_BEFORE_SD) {
            // Prepare a CSV line
            csvLine += String(wifiController.formatCurrentTime(measurement.timeinfo, true, false) + "," +
                              String(measurement.busVolts, 3) + "," +
                              String(measurement.currentMilliAmps, 3) + "\n");
            nbLines++;
        }
        if (nbLines > 0) {
            // Write the CSV line to the file
            file = SD.open(fileName.c_str(), FILE_APPEND);
            file.print(csvLine);
            file.close();
        }
    }
}
