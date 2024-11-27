#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define ADVERTISING_INTERVAL 15000 // 15 seconds in milliseconds

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long lastAdvertisingTime = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
      Serial.println("Connection status: Active");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
      Serial.println("Connection status: Inactive");
      Serial.println("Preparing to restart advertising...");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        Serial.println("New characteristic write received");
        Serial.println("Received value: " + String(value.c_str()));
        Serial.println("Value length: " + String(value.length()));
        
        if (value == "ping") {
          Serial.println("Ping received, sending pong response");
          pCharacteristic->setValue("pong");
          pCharacteristic->notify();
          Serial.println("Pong response sent");
        }
      } else {
        Serial.println("Warning: Received empty characteristic write");
      }
    }
};

void startAdvertising() {
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    BLEAdvertisementData advertisementData;
    advertisementData.setCompleteServices(BLEUUID(SERVICE_UUID));
    advertisementData.setName("XIAO_ESP32C3");
    pAdvertising->setAdvertisementData(advertisementData);
    
    // Optional: Set scan response data
    BLEAdvertisementData scanResponseData;
    scanResponseData.setCompleteServices(BLEUUID(SERVICE_UUID));
    pAdvertising->setScanResponseData(scanResponseData);
    
    BLEDevice::startAdvertising();
    Serial.println("BLE advertising started");
    lastAdvertisingTime = millis();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server...");
  Serial.println("Initializing BLE Device as XIAO_ESP32C3");

  BLEDevice::init("XIAO_ESP32C3");
  Serial.println("BLE Device initialized");

  pServer = BLEDevice::createServer();
  Serial.println("BLE Server created");
  
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("Server callbacks set");

  BLEService *pService = pServer->createService(SERVICE_UUID);
  Serial.println("BLE Service created with UUID: " + String(SERVICE_UUID));

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  Serial.println("BLE Characteristic created with UUID: " + String(CHARACTERISTIC_UUID));

  pCharacteristic->setCallbacks(new MyCallbacks());
  Serial.println("Characteristic callbacks set");
  
  pCharacteristic->addDescriptor(new BLE2902());
  Serial.println("BLE2902 descriptor added");

  pService->start();
  Serial.println("BLE Service started");

  // Configure advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->setMinInterval(0x20);
  pAdvertising->setMaxInterval(0x40);
  Serial.println("Advertising parameters configured");
  
  startAdvertising();
  Serial.println("BLE server ready, waiting for connections...");
}

void loop() {
  unsigned long currentTime = millis();

  // Handle disconnection
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Disconnection detected");
    delay(500);
    startAdvertising();
    Serial.println("Advertising restarted after disconnection");
    oldDeviceConnected = deviceConnected;
  }
  
  // Handle new connection
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("New connection established");
    oldDeviceConnected = deviceConnected;
  }

  // Regular advertising restart if not connected
  if (!deviceConnected && (currentTime - lastAdvertisingTime >= ADVERTISING_INTERVAL)) {
    Serial.println("Periodic advertising restart");
    startAdvertising();
  }

  // Visual indicator for waiting state
  if (!deviceConnected && !oldDeviceConnected) {
    Serial.print(".");
  }
  
  delay(1000);
}