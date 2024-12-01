#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "button_handler.hpp"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define ADVERTISING_INTERVAL 15000

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long lastAdvertisingTime = 0;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Connected");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Disconnected");
  }
};

void startAdvertising() {
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  BLEAdvertisementData advertisementData;
  advertisementData.setCompleteServices(BLEUUID(SERVICE_UUID));
  advertisementData.setName("XIAO_ESP32C3");
  pAdvertising->setAdvertisementData(advertisementData);

  BLEAdvertisementData scanResponseData;
  scanResponseData.setCompleteServices(BLEUUID(SERVICE_UUID));
  pAdvertising->setScanResponseData(scanResponseData);

  BLEDevice::startAdvertising();
  lastAdvertisingTime = millis();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE...");

  BLEDevice::init("XIAO_ESP32C3");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ |
                               BLECharacteristic::PROPERTY_WRITE |
                               BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->setMinInterval(0x20);
  pAdvertising->setMaxInterval(0x40);

  buttonHandlerSetup();

  startAdvertising();
  Serial.println("Ready");
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    startAdvertising();
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  if (!deviceConnected &&
      (millis() - lastAdvertisingTime >= ADVERTISING_INTERVAL)) {
    startAdvertising();
  }

  uint8_t buttonState = getButtonState();
  if (buttonState != 0 && deviceConnected) {
    pCharacteristic->setValue(&buttonState, 1); // Send the byte to receiver (ESP32S3)
    pCharacteristic->notify();
  }

  delay(10);
}