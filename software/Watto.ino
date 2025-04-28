#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <INA.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define DELAY 50

#define SERIAL_SPEED 9600
#define SHUNT_MICRO_OHM 2000  // 2 mOhm
#define MAXIMUM_AMPS 5          // 5A attendu max

INA_Class INA;
uint8_t devicesFound = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(SERIAL_SPEED);
  delay(100); // Petit délai de démarrage
  Serial.println("Watto");
  
  Wire.begin(3, 10, 400000); // SDA=3, SCL=10, 400kHz

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Erreur écran OLED"));
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("INA Scan..."));
  display.display();

  devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);

  while (devicesFound == 0) {
    Serial.println(F("Aucun INA trouvé, retry..."));
    delay(5000);
    devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);
  }

  Serial.print(F("INA trouvés : "));
  Serial.println(devicesFound);

  INA.setBusConversion(8500);             // Conversion time
  INA.setShuntConversion(8500);
  INA.setAveraging(128);
  INA.setMode(INA_MODE_CONTINUOUS_BOTH);   // Mesure continue
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);

  Serial.println(F("INA Measures:"));
  Serial.println(F("Nr Adr Type Bus[V] Shunt[mV] Current[mA] Power[mW]"));
  
  for (uint8_t i = 0; i < devicesFound; i++) {
    float busVolts = INA.getBusMilliVolts(i) / 1000.0;
    //float shuntMilliVolts = INA.getShuntMicroVolts(i) / 1000.0;
    float currentMilliAmps = INA.getBusMicroAmps(i) / 1000.0;
    float powerWatts = INA.getBusMicroWatts(i) / 1000000.0;

    // Affichage Serial
    Serial.print(i + 1);
    Serial.print(F(" 0x"));
    Serial.print(INA.getDeviceAddress(i), HEX);
    Serial.print(F(" "));
    Serial.print(INA.getDeviceName(i));
    Serial.print(F(" "));
    Serial.print(busVolts, 3);
    Serial.print(F("V "));
    Serial.print(F("0.00 mV "));
    Serial.print(currentMilliAmps, 3);
    Serial.print(F("mA "));
    Serial.print(powerWatts, 3);
    Serial.println(F("W"));

    // Affichage OLED
    display.print(F("V:"));
    display.print(busVolts, 2);
    display.println(F("V"));
    display.print(F("I:"));
    display.print(currentMilliAmps, 1);
    display.println(F("mA"));
    display.print(F("P:"));
    display.print(powerWatts, 1);
    display.println(F("W"));
    display.println();
  }

  display.display();
  Serial.println();
  
  delay(DELAY);
}
