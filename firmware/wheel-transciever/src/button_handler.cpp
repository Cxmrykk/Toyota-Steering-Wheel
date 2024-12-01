#include "button_handler.hpp"

// ADC threshold values
const int THRESHOLD_4095 = 4000;
const int THRESHOLD_2600 = 2300;
const int THRESHOLD_2300 = 2000;
const int THRESHOLD_1500 = 1450;
const int THRESHOLD_1450 = 1400;
const int THRESHOLD_1100 = 1000;
const int THRESHOLD_750 = 720;
const int THRESHOLD_720 = 700;
const int THRESHOLD_500 = 480;
const int THRESHOLD_330 = 300;
const int THRESHOLD_250 = 230;

// Enum for button IDs
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

// Variables to store previous states
ButtonID lastA0State = ButtonID::NONE;
ButtonID lastA1State = ButtonID::NONE;
ButtonID lastA2State = ButtonID::NONE;

// Variables to track digital input states
bool lastD10State = HIGH;
bool lastD9State = HIGH;
bool lastD8State = HIGH;

bool confirmADCReading(int pin, int firstReading) {
  delayMicroseconds(100);
  int secondReading = analogRead(pin);
  return abs(secondReading - firstReading) <= 50;
}

void checkA0Buttons() {
  int reading = analogRead(A0);
  ButtonID currentState = lastA0State;

  if (reading <= THRESHOLD_330) {
    if (lastA0State != ButtonID::NONE && confirmADCReading(A0, reading)) {
      currentState = ButtonID::NONE;
    }
  } else if (lastA0State == ButtonID::NONE) {
    if (reading > THRESHOLD_4095 && confirmADCReading(A0, reading))
      currentState = ButtonID::OK;
    else if (reading > THRESHOLD_2300 && confirmADCReading(A0, reading))
      currentState = ButtonID::RETURN;
    else if (reading > THRESHOLD_1450 && confirmADCReading(A0, reading))
      currentState = ButtonID::NEXT_SONG;
    else if (reading > THRESHOLD_1100 && confirmADCReading(A0, reading))
      currentState = ButtonID::PREV_SONG;
    else if (reading > THRESHOLD_720 && confirmADCReading(A0, reading))
      currentState = ButtonID::VOL_UP;
    else if (reading > THRESHOLD_330 && confirmADCReading(A0, reading))
      currentState = ButtonID::VOL_DOWN;
  }

  if (currentState != lastA0State) {
    lastA0State = currentState;
  }
}

void checkA1Buttons() {
  int reading = analogRead(A1);
  ButtonID currentState = lastA1State;

  if (reading <= THRESHOLD_330) {
    if (lastA1State != ButtonID::NONE && confirmADCReading(A1, reading)) {
      currentState = ButtonID::NONE;
    }
  } else if (lastA1State == ButtonID::NONE) {
    if (reading > THRESHOLD_4095 && confirmADCReading(A1, reading))
      currentState = ButtonID::RADAR;
    else if (reading > THRESHOLD_2600 && confirmADCReading(A1, reading))
      currentState = ButtonID::LANE_ASSIST;
    else if (reading > THRESHOLD_1450 && confirmADCReading(A1, reading))
      currentState = ButtonID::LEFT_ARROW;
    else if (reading > THRESHOLD_1100 && confirmADCReading(A1, reading))
      currentState = ButtonID::UP_ARROW;
    else if (reading > THRESHOLD_720 && confirmADCReading(A1, reading))
      currentState = ButtonID::DOWN_ARROW;
    else if (reading > THRESHOLD_330 && confirmADCReading(A1, reading))
      currentState = ButtonID::RIGHT_ARROW;
  }

  if (currentState != lastA1State) {
    lastA1State = currentState;
  }
}

void checkA2Buttons() {
  int reading = analogRead(A2);
  ButtonID currentState = lastA2State;

  if (reading <= THRESHOLD_250) {
    if (lastA2State != ButtonID::NONE && confirmADCReading(A2, reading)) {
      currentState = ButtonID::NONE;
    }
  } else if (lastA2State == ButtonID::NONE) {
    if (reading > THRESHOLD_4095 && confirmADCReading(A2, reading))
      currentState = ButtonID::CRUISE_CONTROL;
    else if (reading > THRESHOLD_2600 && confirmADCReading(A2, reading))
      currentState = ButtonID::CANCEL;
    else if (reading > THRESHOLD_1500 && confirmADCReading(A2, reading))
      currentState = ButtonID::CC_PLUS;
    else if (reading > THRESHOLD_750 && confirmADCReading(A2, reading))
      currentState = ButtonID::CC_MINUS_MODE;
    else if (reading > THRESHOLD_500 && confirmADCReading(A2, reading))
      currentState = ButtonID::PHONE;
    else if (reading > THRESHOLD_250 && confirmADCReading(A2, reading))
      currentState = ButtonID::ASSISTANT;
  }

  if (currentState != lastA2State) {
    lastA2State = currentState;
  }
}

void checkDigitalInputs() {
  bool currentD10State = digitalRead(D10);
  bool currentD9State = digitalRead(D9);
  bool currentD8State = digitalRead(D8);

  lastD10State = currentD10State;
  lastD9State = currentD9State;
  lastD8State = currentD8State;
}

void buttonHandlerSetup() {
  analogReadResolution(12);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  pinMode(D10, INPUT_PULLUP);
  pinMode(D9, INPUT_PULLUP);
  pinMode(D8, INPUT_PULLUP);
}

uint8_t getButtonState() {
  static ButtonID lastState = ButtonID::NONE;
  static bool wasPressed = false;

  checkA0Buttons();
  checkA1Buttons();
  checkA2Buttons();
  checkDigitalInputs();

  ButtonID currentState = ButtonID::NONE;

  if (lastA0State != ButtonID::NONE)
    currentState = lastA0State;
  else if (lastA1State != ButtonID::NONE)
    currentState = lastA1State;
  else if (lastA2State != ButtonID::NONE)
    currentState = lastA2State;
  else if (lastD10State == LOW)
    currentState = ButtonID::PADDLE_LEFT;
  else if (lastD9State == LOW)
    currentState = ButtonID::PADDLE_RIGHT;
  else if (lastD8State == LOW)
    currentState = ButtonID::HORN;

  uint8_t result = 0;

  if (currentState != ButtonID::NONE) {
    if (currentState != lastState || !wasPressed) {
      // Button press event
      result = static_cast<uint8_t>(currentState);
      wasPressed = true;
    }
  } else if (lastState != ButtonID::NONE) {
    // Button release event
    result = static_cast<uint8_t>(lastState) | 0x80;  // Set 8th bit
    wasPressed = false;
  }

  lastState = currentState;
  return result;
}
