#include <Arduino.h>
#include <BLEClient.h>
#include <BLEDevice.h>

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
#define INTERIOR_LIGHT D1
#define INTERIOR_LIGHT_ID 23  // Changed to match new ButtonID enum

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLEClient* pClient;
static bool lastInteriorLightState = false;

enum class ButtonID : uint8_t {
  NONE = 0,
  MODE = 1,
  LEFT = 2,
  NEXT_SONG = 3,
  OK = 4,
  UP = 5,
  PREV_SONG = 6,
  RETURN = 7,
  PHONE = 8,
  DOWN = 9,
  VOLUME_UP = 10,
  ASSISTANT = 11,
  RIGHT = 12,
  VOLUME_DOWN = 13,
  CRUISE_CONTROL = 14,
  CANCEL = 15,
  CC_PLUS = 16,
  CC_MINUS = 17,
  RADAR = 18,
  LANE_ASSIST = 19,
  PADDLE_LEFT = 20,
  PADDLE_RIGHT = 21,
  HORN = 22,
  BACKLIGHT = 23,  // Receive only
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  for (int i = 0; i < length; i++) {
    uint8_t buttonState = pData[i];
    bool buttonReleased = (buttonState & 0x80) != 0;                // Check 8th bit
    ButtonID buttonID = static_cast<ButtonID>(buttonState & 0x7F);  // Mask out 8th bit

    if (buttonReleased) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print(static_cast<uint8_t>(buttonID));
      Serial.println(" released");
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print(static_cast<uint8_t>(buttonID));
      Serial.println(" pressed");
    }
  }
  Serial.println();
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {}

  void onDisconnect(BLEClient* pClient) {
    connected = false;
    Serial.println("Disconnected from server");
    doConnect = false;
    doScan = true;
  }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = false;
    }
  }
};

bool connectToServer() {
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  unsigned long connectStartTime = millis();
  const unsigned long CONNECTION_TIMEOUT = 5000;

  while (!pClient->isConnected() && (millis() - connectStartTime) < CONNECTION_TIMEOUT) {
    pClient->connect(myDevice);
    delay(500);
  }

  if (!pClient->isConnected()) {
    Serial.println("Connection timed out");
    return false;
  }

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

  connected = true;
  return true;
}

void startScan() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0, false);
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("XIAO_ESP32S3_CLIENT");  // Changed client name

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(INTERIOR_LIGHT, INPUT_PULLUP);

  startScan();
}

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

  if (!connected && doScan) {
    startScan();
    doScan = false;
  }

  if (connected) {
    bool currentInteriorLightState = digitalRead(INTERIOR_LIGHT) == LOW;
    if (currentInteriorLightState != lastInteriorLightState) {
      uint8_t stateToSend = INTERIOR_LIGHT_ID;
      if (!currentInteriorLightState) {
        stateToSend |= 0x80;  // Set 8th bit if released
      }
      pRemoteCharacteristic->writeValue(&stateToSend, 1);
      lastInteriorLightState = currentInteriorLightState;
    }

    if (pRemoteCharacteristic->canNotify()) {
      pRemoteCharacteristic->registerForNotify(notifyCallback);
    }
  }

  delay(10);
}