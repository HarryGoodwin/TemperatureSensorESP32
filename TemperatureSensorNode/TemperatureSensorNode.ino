#include "LiquidCrystal.h"
#include <dht11.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define DHT11PIN 13
#define SERVICE_UUID        "181A"
#define TEMPERATURE_CHARACTERISTIC_UUID "2A6E"
#define HUMIDITY_CHARACTERISTIC_UUID "2A6F"

dht11 DHT11;
LiquidCrystal lcd(32, 33, 25, 26, 27, 14);
int humidity, temperature, lowTemp, highTemp, lowHumidity, highHumidity;

int failCount, successCount;

BLECharacteristic *tempCharacteristic;
BLECharacteristic *humidityCharacteristic;

void setupLCD() {
  lcd.setCursor(0, 0);
  lcd.begin(20, 4);
  lcd.print("Starting...");
}

void setupBT() {
  BLEDevice::init("Temperature Reporter");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  tempCharacteristic = pService->createCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  humidityCharacteristic = pService->createCharacteristic(HUMIDITY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); 
  BLEDevice::startAdvertising();
}

void setup() {
  setupLCD();
  setupBT();
  lowTemp = INT_MAX;
  highTemp = INT_MIN;
  lowHumidity = INT_MAX;
  highHumidity = INT_MIN;


  failCount = 0;
  successCount = 0;
}

void updateBluetooth() {
  tempCharacteristic->setValue(String(temperature));
  tempCharacteristic->notify();
  humidityCharacteristic->setValue(String(humidity));
  humidityCharacteristic->notify();
}

void testHighLowValues() {
  highTemp = max(temperature, highTemp);
  lowTemp = min(temperature, lowTemp);

  highHumidity = max(humidity, highHumidity);
  lowHumidity = min(humidity, lowHumidity);
}

void updateLCD() {
  lcd.setCursor(0, 0);
  String tempStr =  "Temperature: " + String(temperature) + "\xDF" + "C";
  lcd.print(tempStr);

  lcd.setCursor(0, 1);
  String lowHighTempStr =  "Low: " + String(lowTemp) +  + "\xDF" + "C" + ", High: " + String(highTemp) + "\xDF" + "C";
  lcd.print(lowHighTempStr);

  lcd.setCursor(0, 2);
  String humidityStr =  "Humidity: " + String(humidity) + "%";
  lcd.print(humidityStr);

  lcd.setCursor(0, 3);
  String lowHighHumidityStr =  "Low: " + String(lowHumidity) + "%" + ", High: " + String(highHumidity) + "%";
  lcd.print(lowHighHumidityStr);
}

void updateTempLCD() {
  lcd.setCursor(0, 0);
  String sStr =  "Success: " + String(successCount);
  lcd.print(sStr);

  lcd.setCursor(0, 1);
  String fStr =  "Fail: " + String(failCount);
  lcd.print(fStr);
  
  lcd.setCursor(0, 2);
  lcd.print("Boot time: " + String(millis()));
}

String checkError(int status) {
   String error = "";
   switch (status) {
    case DHTLIB_ERROR_CHECKSUM:
      error = "Checksum error";
      failCount += 1;
      break;
    case DHTLIB_ERROR_TIMEOUT: 
      error = "Timeout error";
      failCount += 1;
      break;
    default: 
      successCount += 1;
      return "";
   }
}

void loop() {
  delay(30000);
  int status = DHT11.read(DHT11PIN);

  String errorMessage = checkError(status);

  // Early return from loop if error.
  if (errorMessage.length() > 0) {
    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print(errorMessage);

    updateTempLCD();
    return;
  }

  // Update local values from DHT11.
  temperature = (int)DHT11.temperature;
  humidity = (int)DHT11.humidity;

  testHighLowValues();
  updateTempLCD();
  updateBluetooth();
}

