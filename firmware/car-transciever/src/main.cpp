#include <Arduino.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <SPI.h>

#include "remote.hpp"

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

// use hardware SPI
// implement remote logic

#define PIN_INTERIOR D0
#define PIN_HORN D1
#define PIN_AUX_LATCH D2
#define PIN_SR_LATCH D3
#define PIN_CLOCK SCK
#define PIN_DATA MOSI

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLEClient* pClient;
static bool lastInteriorLightState = false;

// Number of shift registers
const int NUM_REGISTERS = 3;
const int NUM_OUTPUTS = NUM_REGISTERS * 8;  // Total number of outputs (8 outputs per register * 3 registers)

// Array to keep track of current state
byte registerStates[NUM_REGISTERS] = {0};

// Function to clear all registers (remains the same)
void clearRegisters() {
  for (int i = 0; i < NUM_REGISTERS; i++) {
    registerStates[i] = 0;
  }
}

// Function to write data to all registers using hardware SPI
void writeRegisters() {
  // Start the SPI transaction (20 Mhz)
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));

  // Pull latch pin low to start transfer
  digitalWrite(PIN_SR_LATCH, LOW);

  // Transfer data for each register
  // Note: We transfer from last register to first (MSB first)
  for (int i = NUM_REGISTERS - 1; i >= 0; i--) {
    SPI.transfer(registerStates[i]);
  }

  // Pull latch pin high to update outputs
  digitalWrite(PIN_SR_LATCH, HIGH);

  // End the SPI transaction
  SPI.endTransaction();
}

// Function to set a specific output (remains the same)
void setOutput(int output, bool state) {
  if (output < 0 || output >= NUM_OUTPUTS) return;

  int registerIndex = output / 8;
  int bitIndex = output % 8;

  if (state) {
    registerStates[registerIndex] |= (1 << bitIndex);
  } else {
    registerStates[registerIndex] &= ~(1 << bitIndex);
  }

  writeRegisters();
}

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  for (int i = 0; i < length; i++) {
    uint8_t buttonState = pData[i];
    uint8_t buttonValue = buttonState & 0x7F;
    bool buttonReleased = (buttonState & 0x80) != 0;         // Check 8th bit
    ButtonID buttonID = static_cast<ButtonID>(buttonValue);  // Mask out 8th bit

    if (buttonReleased) {
      digitalWrite(LED_BUILTIN, HIGH);

      // Turn off shift register output
      if (buttonValue <= 21) {
        setOutput(buttonValue, false);
      }

      // Turn off horn
      if (buttonValue == 22) {
        digitalWrite(PIN_HORN, LOW);
      }

      // Set remote output state for button
      setPWMForButton(buttonID, false);

      Serial.print(buttonValue);
      Serial.println(" released");
    } else {
      digitalWrite(LED_BUILTIN, LOW);

      // Turn on shift register output
      if (buttonValue <= 21) {
        setOutput(buttonValue, true);
      }

      // Turn on horn
      if (buttonValue == 22) {
        digitalWrite(PIN_HORN, HIGH);
      }

      // Set remote output state for button
      setPWMForButton(buttonID, true);

      Serial.print(buttonValue);
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
  BLEDevice::init("XIAO_ESP32S3_CLIENT");

  // Initialise SPI
  SPI.begin();

  // Set the LED to default state (on)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialise other pins
  pinMode(PIN_INTERIOR, INPUT);
  pinMode(PIN_HORN, OUTPUT);
  pinMode(PIN_AUX_LATCH, OUTPUT);
  pinMode(PIN_SR_LATCH, OUTPUT);

  // Initially disable the horn
  digitalWrite(PIN_HORN, LOW);

  // Initially clear all registers
  clearRegisters();
  writeRegisters();

  // Start BLE scanning
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
    bool currentInteriorLightState = digitalRead(PIN_INTERIOR) == LOW;
    if (currentInteriorLightState != lastInteriorLightState) {
      uint8_t stateToSend = static_cast<uint8_t>(ButtonID::BACKLIGHT);
      if (!currentInteriorLightState) {
        stateToSend |= 0x80;  // Set 8th bit if turning off
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