/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

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

#define DELAY 100

#define SERIAL_SPEED 115200

#define INA_SHUNT_MICRO_OHM 2000                       // 2 mOhm
#define INA_MAXIMUM_AMPS 5                             // 5A attendu max
#define INA_BUS_CONVERSION_TIME 8500                   // Conversion time for bus voltage
#define INA_SHUNT_CONVERSION_TIME 8500                 // Conversion time for shunt voltage
#define INA_AVERAGING_COUNT 128                        // Averaging count
#define INA_MEASUREMENT_MODE INA_MODE_CONTINUOUS_BOTH  // Measurement mode

#define I2C_SDA_PIN 3           // SDA pin
#define I2C_SCL_PIN 10          // SCL pin
#define I2C_CLOCK_SPEED 400000  // I2C clock speed in Hz

#define TIMEZONE 1
#define DAYLIGHT_SAVING 1

#define DATA_FILE "/data.csv"  // Nom du fichier CSV

INAManager inaManager;
DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SDManager sdManager;
WiFiController wifiController(WIFI_SSID, WIFI_PASSWORD, TIMEZONE, DAYLIGHT_SAVING);
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
    // Initialisation de la communication série et I2C
    Serial.begin(SERIAL_SPEED);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_SPEED);

    delay(100);  // Petit délai de démarrage
    Serial.println("Watto");

    // Initialisation de l'écran OLED
    if (!displayManager.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
        Serial.println(F("Erreur écran OLED"));
        while (1);
    }

    // Initialisation de l'INA
    displayManager.showMessage(F("Seaching INA..."));
    if (!inaManager.begin(INA_MAXIMUM_AMPS, INA_SHUNT_MICRO_OHM)) {
        Serial.println(F("Aucun INA trouvé, retry..."));
        while (1);
    }
    inaManager.configure(INA_BUS_CONVERSION_TIME, INA_SHUNT_CONVERSION_TIME, INA_AVERAGING_COUNT, INA_MEASUREMENT_MODE);
    displayManager.showMessage(F("INA found."));

    // Connect to WiFi and sync time
    displayManager.showMessage(F("Connect to WiFi..."));
    wifiController.connect(configModeCallback);
    displayManager.showMessage(F("Sync time..."));
    wifiController.syncTime();

    // Initialize SD card
    if (!sdManager.begin()) {
        Serial.println("Failed to initialize SD card");
        while (1);
    }

    // Démarrer le serveur web
    apiManager.begin();
}

void loop() {
    // Récupérer les informations Wi-Fi
    String wifiSSID = wifiController.getSSID();
    String wifiIP = wifiController.getIPAddress();

    // Afficher les informations Wi-Fi sur l'écran
    displayManager.clear();
    displayManager.showWiFiInfo(wifiSSID, wifiIP);

    float busVolts = inaManager.getBusVolts();
    float currentMilliAmps = inaManager.getCurrentMilliAmps();
    float powerWatts = inaManager.getPowerWatts();

    // Get current time
    struct tm timeinfo = wifiController.getCurrentTime();

    // Display measurements on OLED
    displayManager.showMeasurements(busVolts, currentMilliAmps, powerWatts, wifiController.formatCurrentTime(timeinfo, false, false), inaManager.getDeviceAddress());

    // Vérifier si les mesures doivent être effectuées
    if (apiManager.isMeasuring()) {
        if (fileName == "") {
            deviceName = apiManager.getDeviceName();
            fileName = "/" + deviceName + "_" + wifiController.formatCurrentTime(timeinfo, false, true) + ".csv";
            // Prepare the CSV file
            sdManager.writeFile(fileName.c_str(), "Timestamp,Address,BusVolts,CurrentMilliAmps,PowerWatts\n");

            Serial.println("Measuring for device: " + deviceName);
        }

        // Prepare CSV line
        csvLine = String(wifiController.formatCurrentTime(timeinfo, true, false) + "," + String(inaManager.getDeviceAddress(), HEX) + "," + String(busVolts, 3) + "," + String(currentMilliAmps, 3) + "," + String(powerWatts, 3) + "\n");
        // Write to data.csv
        sdManager.appendFile(fileName.c_str(), csvLine.c_str());
    } else {
        fileName = "";
    }
    delay(DELAY);
}
