#include <Arduino.h>
#include <BLEClient.h>
#include <BLEDevice.h>

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

enum class ButtonID : uint8_t {
  NONE = 0,
  OK = 1,
  RETURN = 2,
  NEXT_SONG = 3,
  PREV_SONG = 4,
  VOL_UP = 5,
  VOL_DOWN = 6,
  RADAR = 7,
  LANE_ASSIST = 8,
  LEFT_ARROW = 9,
  UP_ARROW = 10,
  DOWN_ARROW = 11,
  RIGHT_ARROW = 12,
  CRUISE_CONTROL = 13,
  CANCEL = 14,
  CC_PLUS = 15,
  CC_MINUS_MODE = 16,
  PHONE = 17,
  ASSISTANT = 18,
  PADDLE_LEFT = 19,
  PADDLE_RIGHT = 20,
  HORN = 21
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                           uint8_t* pData, size_t length, bool isNotify) {
  Serial.print("Notification received: ");
  for (int i = 0; i < length; i++) {
    uint8_t buttonState = pData[i];
    bool buttonReleased = (buttonState & 0x80) != 0;  // Check 8th bit
    ButtonID buttonID =
        static_cast<ButtonID>(buttonState & 0x7F);  // Mask out 8th bit

    if (buttonReleased) {
      Serial.print(static_cast<uint8_t>(buttonID));
      Serial.println(" released");
    } else {
      Serial.print(static_cast<uint8_t>(buttonID));
      Serial.println(" pressed");
    }
  }
  Serial.println();
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = false;
    }
  }
};

bool connectToServer() {
  BLEClient* pClient = BLEDevice::createClient();
  pClient->connect(myDevice);

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}

void startScan() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("XIAO_ESP32S3");
  startScan();
}

unsigned long lastScanTime = 0;
const unsigned long SCAN_INTERVAL = 15000;

void loop() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("Connected");
    } else {
      Serial.println("Connection failed");
      doScan = true;
    }
    doConnect = false;
  }

  if (!connected) {
    unsigned long currentTime = millis();
    if (currentTime - lastScanTime >= SCAN_INTERVAL) {
      startScan();
      lastScanTime = currentTime;
    }
  }

  delay(10);
}