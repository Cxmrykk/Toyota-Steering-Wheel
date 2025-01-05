#include <Arduino.h>
#include <BLE2902.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// BLE Configuration
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define ADVERTISING_INTERVAL 2000  // Increased to 2 seconds

// Pin Definitions and ADC Configuration
#define ADC_VERIFY_READ_DELAY 100  // microseconds
#define ADC_VERIFY_READ_RANGE 50
// Average
#define ADC_AVERAGE_SAMPLES 100
#define ADC_SAMPLE_DELAY_US 50

#define PIN_A0 A0
#define PIN_A1 A1
#define PIN_HORN D2
#define PIN_PADDLE_RIGHT D3
#define PIN_PADDLE_LEFT D4
#define PIN_BACKLIGHT D5

// BLE Variables
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long lastAdvertisingTime = 0;
unsigned long lastBacklightToggleTime = 0;
bool backlightState = false;
unsigned long errorCount = 0;
const unsigned long ERROR_THRESHOLD = 5;

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

namespace Threshold {
// Resting state values
constexpr uint16_t A0_RESTING = 3500;
constexpr uint16_t A1_RESTING = 4000;

// A0 thresholds (values must be BELOW these to trigger)
constexpr uint16_t A0_VOLUME_DOWN = 1890;
constexpr uint16_t A0_RIGHT = 1860;
constexpr uint16_t A0_ASSISTANT = 1820;
constexpr uint16_t A0_VOLUME_UP = 980;
constexpr uint16_t A0_DOWN = 900;
constexpr uint16_t A0_PHONE = 850;
constexpr uint16_t A0_RETURN = 550;
constexpr uint16_t A0_PREV_SONG = 480;
constexpr uint16_t A0_UP = 400;
constexpr uint16_t A0_OK = 280;
constexpr uint16_t A0_NEXT_SONG = 180;
constexpr uint16_t A0_LEFT = 80;
constexpr uint16_t A0_MODE = 0;

// A1 thresholds (values must be BELOW these to trigger)
constexpr uint16_t A1_LANE_ASSIST = 1980;
constexpr uint16_t A1_RADAR = 1880;
constexpr uint16_t A1_CC_MINUS = 1600;
constexpr uint16_t A1_CC_PLUS = 820;
constexpr uint16_t A1_CANCEL = 320;
constexpr uint16_t A1_CRUISE_CONTROL = 0;
}  // namespace Threshold

// Button State Variables
ButtonID previousA0 = ButtonID::NONE;
ButtonID previousA1 = ButtonID::NONE;
bool previousHorn = false;
bool previousPaddleR = false;
bool previousPaddleL = false;

// Analog reset variables (prevents noise in some cases)
bool a0WaitingForReset = false;
bool a1WaitingForReset = false;

void checkAndResetBLE() {
  errorCount++;
  if (errorCount >= ERROR_THRESHOLD) {
    Serial.println("Too many BLE errors, restarting BLE stack");
    BLEDevice::deinit(true);
    delay(1000);
    setup();  // Reinitialize BLE
    errorCount = 0;
  }
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    errorCount = 0;  // Reset error count on successful connection
    Serial.println("Connected");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Disconnected");
    delay(500);
    BLEDevice::getAdvertising()->stop();
  }
};

void startAdvertising() {
  // Stop any existing advertising first
  BLEDevice::getAdvertising()->stop();
  delay(100);  // Give it time to stop completely

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();

  // Set new advertising data
  BLEAdvertisementData advertisementData;
  advertisementData.setCompleteServices(BLEUUID(SERVICE_UUID));
  advertisementData.setName("XIAO_ESP32S3_WHEEL");
  pAdvertising->setAdvertisementData(advertisementData);

  // Start advertising with specific parameters
  pAdvertising->start();
  lastAdvertisingTime = millis();
}

bool confirmADCReading(int pin, int firstReading) {
  delayMicroseconds(ADC_VERIFY_READ_DELAY);
  int secondReading = analogRead(pin);
  return abs(secondReading - firstReading) <= ADC_VERIFY_READ_RANGE;
}

uint16_t getAveragedADCReading(int pin) {
  uint32_t sum = 0;
  for (int i = 0; i < ADC_AVERAGE_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(ADC_SAMPLE_DELAY_US);
  }
  return sum / ADC_AVERAGE_SAMPLES;
}

ButtonID getA0() {
  uint16_t valueA0 = analogRead(PIN_A0);
  Serial.println(valueA0);

  // If waiting for reset, check if value has returned above resting threshold
  if (a0WaitingForReset) {
    if (valueA0 >= Threshold::A0_RESTING) {
      a0WaitingForReset = false;
      return ButtonID::NONE;
    }
    return previousA0;
  }

  // Check if A0 is significantly below resting state
  if (valueA0 < (Threshold::A0_RESTING)) {
    if (confirmADCReading(PIN_A0, valueA0)) {
      // Get averaged reading
      valueA0 = getAveragedADCReading(PIN_A0);
      ButtonID result = ButtonID::NONE;

      if (valueA0 > Threshold::A0_VOLUME_DOWN)
        result = ButtonID::VOLUME_DOWN;
      else if (valueA0 > Threshold::A0_RIGHT)
        result = ButtonID::RIGHT;
      else if (valueA0 > Threshold::A0_ASSISTANT)
        result = ButtonID::ASSISTANT;
      else if (valueA0 > Threshold::A0_VOLUME_UP)
        result = ButtonID::VOLUME_UP;
      else if (valueA0 > Threshold::A0_DOWN)
        result = ButtonID::DOWN;
      else if (valueA0 > Threshold::A0_PHONE)
        result = ButtonID::PHONE;
      else if (valueA0 > Threshold::A0_RETURN)
        result = ButtonID::RETURN;
      else if (valueA0 > Threshold::A0_PREV_SONG)
        result = ButtonID::PREV_SONG;
      else if (valueA0 > Threshold::A0_UP)
        result = ButtonID::UP;
      else if (valueA0 > Threshold::A0_OK)
        result = ButtonID::OK;
      else if (valueA0 > Threshold::A0_NEXT_SONG)
        result = ButtonID::NEXT_SONG;
      else if (valueA0 > Threshold::A0_LEFT)
        result = ButtonID::LEFT;
      else if (valueA0 >= Threshold::A0_MODE)
        result = ButtonID::MODE;

      if (result != ButtonID::NONE) {
        a0WaitingForReset = true;
        return result;
      }
    }
  }

  return ButtonID::NONE;
}

ButtonID getA1() {
  uint16_t valueA1 = analogRead(PIN_A1);

  // If waiting for reset, check if value has returned above resting threshold
  if (a1WaitingForReset) {
    if (valueA1 >= Threshold::A1_RESTING) {
      a1WaitingForReset = false;
      return ButtonID::NONE;
    }
    return previousA1;
  }

  // Check if A1 is significantly below resting state
  if (valueA1 < (Threshold::A1_RESTING)) {
    if (confirmADCReading(PIN_A1, valueA1)) {
      // Get averaged reading
      valueA1 = getAveragedADCReading(PIN_A1);
      ButtonID result = ButtonID::NONE;

      if (valueA1 > Threshold::A1_LANE_ASSIST)
        result = ButtonID::LANE_ASSIST;
      else if (valueA1 > Threshold::A1_RADAR)
        result = ButtonID::RADAR;
      else if (valueA1 > Threshold::A1_CC_MINUS)
        result = ButtonID::CC_MINUS;
      else if (valueA1 > Threshold::A1_CC_PLUS)
        result = ButtonID::CC_PLUS;
      else if (valueA1 > Threshold::A1_CANCEL)
        result = ButtonID::CANCEL;
      else if (valueA1 >= Threshold::A1_CRUISE_CONTROL)
        result = ButtonID::CRUISE_CONTROL;

      if (result != ButtonID::NONE) {
        a1WaitingForReset = true;
        return result;
      }
    }
  }

  return ButtonID::NONE;
}

bool getHorn() { return digitalRead(PIN_HORN) == LOW; }

bool getPaddleR() { return digitalRead(PIN_PADDLE_RIGHT) == LOW; }

bool getPaddleL() { return digitalRead(PIN_PADDLE_LEFT) == LOW; }

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  // Pin Setup
  pinMode(PIN_A0, INPUT);
  pinMode(PIN_A1, INPUT);
  pinMode(PIN_HORN, INPUT);
  pinMode(PIN_PADDLE_RIGHT, INPUT);
  pinMode(PIN_PADDLE_LEFT, INPUT);
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, LOW);

  // BLE Setup
  Serial.println("Starting BLE...");
  BLEDevice::init("XIAO_ESP32S3_WHEEL");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  // Start advertising
  startAdvertising();
  Serial.println("Ready");
}

void handleButtonStateChange(bool currentState, bool& previousState, ButtonID buttonId) {
  if (currentState != previousState) {
    uint8_t value = static_cast<uint8_t>(buttonId);
    if (previousState) {
      value |= 0x80;  // off flag
    }
    if (currentState || previousState) {  // only send if there's a change to report
      pCharacteristic->setValue(&value, 1);
      pCharacteristic->notify();
    }
    previousState = currentState;
  }
}

void handleButtonIDStateChange(ButtonID currentButton, ButtonID& previousButton) {
  if (currentButton != previousButton) {
    if (previousButton != ButtonID::NONE) {
      uint8_t value = static_cast<uint8_t>(previousButton) | 0x80;  // off flag
      pCharacteristic->setValue(&value, 1);
      pCharacteristic->notify();
    }
    if (currentButton != ButtonID::NONE) {
      uint8_t value = static_cast<uint8_t>(currentButton);
      pCharacteristic->setValue(&value, 1);
      pCharacteristic->notify();
    }
    previousButton = currentButton;
  }
}

void loop() {
  // Handle BLE connection state
  if (!deviceConnected && oldDeviceConnected) {
    pServer->startAdvertising();  // Restart advertising
    oldDeviceConnected = deviceConnected;
    lastAdvertisingTime = millis();
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  // Only restart advertising if enough time has passed and we're not connected
  if (!deviceConnected && (millis() - lastAdvertisingTime >= ADVERTISING_INTERVAL)) {
    try {
      startAdvertising();
    } catch (...) {
      checkAndResetBLE();
      lastAdvertisingTime = millis() + ADVERTISING_INTERVAL;
    }
  }

  if (deviceConnected) {
    // Handle button states
    ButtonID currentA0 = getA0();
    ButtonID currentA1 = getA1();
    bool currentHorn = getHorn();
    bool currentPaddleR = getPaddleR();
    bool currentPaddleL = getPaddleL();

    handleButtonIDStateChange(currentA0, previousA0);
    handleButtonIDStateChange(currentA1, previousA1);
    handleButtonStateChange(currentHorn, previousHorn, ButtonID::HORN);
    handleButtonStateChange(currentPaddleR, previousPaddleR, ButtonID::PADDLE_RIGHT);
    handleButtonStateChange(currentPaddleL, previousPaddleL, ButtonID::PADDLE_LEFT);

    // get backlight state if data available
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      uint8_t receivedState = value[0];
      if ((receivedState & 0x7F) == static_cast<int>(ButtonID::BACKLIGHT)) {
        backlightState = (receivedState & 0x80) >> 7;
        digitalWrite(PIN_BACKLIGHT, !backlightState);  // 1 is off, 0 is on (using received flag)
      }
    }
  }

  // Handle backlight behavior
  if (!deviceConnected) {
    if (millis() - lastBacklightToggleTime >= 500) {
      backlightState = !backlightState;
      digitalWrite(PIN_BACKLIGHT, backlightState);
      lastBacklightToggleTime = millis();
    }
  }

  delay(100);
}