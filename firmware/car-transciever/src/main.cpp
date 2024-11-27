#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    String message = "\n-------- NOTIFICATION CALLBACK --------\n";
    message += "Data length: " + String(length) + "\n";
    message += "Notification flag: " + String(isNotify) + "\n";
    message += "Received data: ";
    
    for(int i = 0; i < length; i++) {
        message += (char)pData[i];
    }
    message += "\n";
    
    message += "Raw bytes:\n";
    for(int i = 0; i < length; i++) {
        message += String(pData[i], HEX) + " ";
    }
    message += "\n";
    message += "-------------------------------------\n";
    
    Serial.print(message);
    Serial.flush();
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.println("-------- DEVICE FOUND --------");
    Serial.println("Device address: ");
    Serial.println(advertisedDevice.getAddress().toString().c_str());
    Serial.println("Device name: ");
    Serial.println(advertisedDevice.getName().c_str());
    Serial.println("RSSI: ");
    Serial.println(advertisedDevice.getRSSI());
    
    if (advertisedDevice.haveServiceUUID()) {
      Serial.println("Has service UUID: ");
      Serial.println(advertisedDevice.getServiceUUID().toString().c_str());
      
      if (advertisedDevice.isAdvertisingService(serviceUUID)) {
        Serial.println("Found device with matching service UUID!");
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = false;
      }
    }
    Serial.println("-----------------------------");
  }
};

bool connectToServer() {
  Serial.println("-------- CONNECTING TO SERVER --------");
  Serial.println("Target device address: ");
  Serial.println(myDevice->getAddress().toString().c_str());
  
  BLEClient* pClient = BLEDevice::createClient();
  Serial.println("BLE client created");
  
  pClient->connect(myDevice);
  Serial.println("Connected to server");

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.println("ERROR: Failed to find service UUID");
    Serial.println("Disconnecting...");
    pClient->disconnect();
    return false;
  }
  Serial.println("Service found");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("ERROR: Failed to find characteristic UUID");
    Serial.println("Disconnecting...");
    pClient->disconnect();
    return false;
  }
  Serial.println("Characteristic found");

  if(pRemoteCharacteristic->canNotify()) {
    Serial.println("Registering for notifications");
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  } else {
    Serial.println("WARNING: Characteristic does not support notifications");
  }

  connected = true;
  Serial.println("------------------------------------");
  return true;
}

// New helper function to start scanning
void startScan() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  Serial.println("Starting BLE scan...");
  pBLEScan->start(5, false);  // Scan for 5 seconds
  Serial.println("-----------------------------------");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n-------- STARTING BLE CLIENT --------");
  Serial.println("Target Service UUID: ");
  Serial.println(serviceUUID.toString().c_str());
  Serial.println("Target Characteristic UUID: ");
  Serial.println(charUUID.toString().c_str());
  
  BLEDevice::init("XIAO_ESP32S3");
  Serial.println("BLE Device initialized");

  // Initial scan
  startScan();
}

// Add a variable to track the last scan time
unsigned long lastScanTime = 0;
const unsigned long SCAN_INTERVAL = 15000; // 15 seconds in milliseconds

void loop() {
  if (doConnect) {
    Serial.println("Attempting to connect...");
    if (connectToServer()) {
      Serial.println("SUCCESS: Connected to BLE Server");
    } else {
      Serial.println("FAILED: Could not connect to server");
      doScan = true;  // Enable scanning again if connection fails
    }
    doConnect = false;
  }

  if (connected) {
    Serial.println("-------- SENDING PING --------");
    pRemoteCharacteristic->writeValue("ping");
    Serial.println("Sent: ping");
    Serial.println("----------------------------");
    delay(2000);
  } else {
    // If not connected and it's time for a new scan
    unsigned long currentTime = millis();
    if (currentTime - lastScanTime >= SCAN_INTERVAL) {
      Serial.println("\nRetrying scan...");
      startScan();
      lastScanTime = currentTime;
    }
  }
  
  if (!doConnect && !connected) {
    Serial.print(".");
  }

  delay(1000);
}