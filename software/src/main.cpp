/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>
#include "DisplayManager.h"
#include "INAManager.h"
#include "SDManager.h"
#include "WiFiManager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_I2C_ADDR 0x3C

#define DELAY 100

#define SERIAL_SPEED 115200

#define INA_SHUNT_MICRO_OHM 2000    // 2 mOhm
#define INA_MAXIMUM_AMPS 5          // 5A attendu max
#define INA_BUS_CONVERSION_TIME 8500       // Conversion time for bus voltage
#define INA_SHUNT_CONVERSION_TIME 8500    // Conversion time for shunt voltage
#define INA_AVERAGING_COUNT 128           // Averaging count
#define INA_MEASUREMENT_MODE INA_MODE_CONTINUOUS_BOTH // Measurement mode

#define I2C_SDA_PIN 3               // SDA pin
#define I2C_SCL_PIN 10              // SCL pin
#define I2C_CLOCK_SPEED 400000  // I2C clock speed in Hz

#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define TIMEZONE 1
#define DAYLIGHT_SAVING 1

#define DATA_FILE "/data.csv"       // Nom du fichier CSV

INAManager inaManager;
DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SDManager sdManager;
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD, TIMEZONE, DAYLIGHT_SAVING);

void setup() {
  // Initialisation de la communication série et I2C
  Serial.begin(SERIAL_SPEED);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_SPEED);

  delay(100); // Petit délai de démarrage
  Serial.println("Watto");
  
  // Initialisation de l'écran OLED
  if (!displayManager.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println(F("Erreur écran OLED"));
    while (1);
  }
  displayManager.showStartupMessage();

  // Initialisation de l'INA
  if (!inaManager.begin(INA_MAXIMUM_AMPS, INA_SHUNT_MICRO_OHM)) {
    Serial.println(F("Aucun INA trouvé, retry..."));
    while (1);
  }
  inaManager.configure(INA_BUS_CONVERSION_TIME, INA_SHUNT_CONVERSION_TIME, INA_AVERAGING_COUNT, INA_MEASUREMENT_MODE);

  // Connect to WiFi and sync time
  wifiManager.connect();
  wifiManager.syncTime();

  // Initialize SD card
  if (!sdManager.begin()) {
    Serial.println("Failed to initialize SD card");
    while (1);
  }

  // Reinitialize the DATA_FILE file
  sdManager.writeFile(DATA_FILE, "Timestamp,Address,BusVolts,CurrentMilliAmps,PowerWatts\n");
}

void loop() {
  uint8_t devicesFound = inaManager.getDevicesFound();
  for (uint8_t i = 0; i < devicesFound; i++) {
    float busVolts = inaManager.getBusVolts(i);
    float currentMilliAmps = inaManager.getCurrentMilliAmps(i);
    float powerWatts = inaManager.getPowerWatts(i);

    // Get current time
    String currentTime = wifiManager.getCurrentTime(true);
    // Prepare CSV line
    String csvLine = String(currentTime + "," + String(inaManager.getDeviceAddress(i), HEX) + "," + String(busVolts, 3) + "," + String(currentMilliAmps, 3) + "," + String(powerWatts, 3) + "\n");
    // Write to data.csv
    sdManager.appendFile(DATA_FILE, csvLine.c_str());

    // Display measurements on Serial Monitor
    Serial.printf("INA Measures: Nr=%d Adr=0x%X Type=%s Bus=%.3fV Current=%.3fmA Power=%.3fW\n", 
      i + 1, inaManager.getDeviceAddress(i), inaManager.getDeviceName(i), busVolts, currentMilliAmps, powerWatts);
    // Display measurements on OLED
    currentTime = wifiManager.getCurrentTime(false);
    displayManager.showMeasurements(busVolts, currentMilliAmps, powerWatts, currentTime, inaManager.getDeviceAddress(i));
  }

  // Read and display the content of data.csv
  Serial.println("Content of data.csv:");
  sdManager.readFile(DATA_FILE);
  delay(DELAY);
}
