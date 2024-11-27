#include "button_handler.hpp"

// ADC threshold values
const int THRESHOLD_4095 = 4000;  // Allowance for voltage drop
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

// Enum for A0 buttons
enum class A0Button {
  NONE,
  OK,
  RETURN,
  NEXT_SONG,
  PREV_SONG,
  VOL_UP,
  VOL_DOWN
};

// Enum for A1 buttons
enum class A1Button {
  NONE,
  RADAR,
  LANE_ASSIST,
  LEFT_ARROW,
  UP_ARROW,
  DOWN_ARROW,
  RIGHT_ARROW
};

// Enum for A2 buttons
enum class A2Button {
  NONE,
  CRUISE_CONTROL,
  CANCEL,
  CC_PLUS,
  CC_MINUS_MODE,
  PHONE,
  ASSISTANT
};

// Variables to store previous states
A0Button lastA0State = A0Button::NONE;
A1Button lastA1State = A1Button::NONE;
A2Button lastA2State = A2Button::NONE;

// Variables to track digital input states
bool lastD10State = HIGH;  // HIGH = released, LOW = pressed
bool lastD9State = HIGH;
bool lastD8State = HIGH;

// Function to return A0 button state as String
String getA0ButtonString(A0Button state) {
  switch (state) {
    case A0Button::OK:
      return "A0_OK";
    case A0Button::RETURN:
      return "A0_RETURN";
    case A0Button::NEXT_SONG:
      return "A0_NEXT_SONG";
    case A0Button::PREV_SONG:
      return "A0_PREV_SONG";
    case A0Button::VOL_UP:
      return "A0_VOL_UP";
    case A0Button::VOL_DOWN:
      return "A0_VOL_DOWN";
    case A0Button::NONE:
      return "";  // Return empty string for no button pressed
    default:
      return "";
  }
}

// Function to return A1 button state as String
String getA1ButtonString(A1Button state) {
  switch (state) {
    case A1Button::RADAR:
      return "A1_RADAR";
    case A1Button::LANE_ASSIST:
      return "A1_LANE_ASSIST";
    case A1Button::LEFT_ARROW:
      return "A1_LEFT_ARROW";
    case A1Button::UP_ARROW:
      return "A1_UP_ARROW";
    case A1Button::DOWN_ARROW:
      return "A1_DOWN_ARROW";
    case A1Button::RIGHT_ARROW:
      return "A1_RIGHT_ARROW";
    case A1Button::NONE:
      return "";  // Return empty string for no button pressed
    default:
      return "";
  }
}

// Function to return A2 button state as String
String getA2ButtonString(A2Button state) {
  switch (state) {
    case A2Button::CRUISE_CONTROL:
      return "A2_CRUISE_CONTROL";
    case A2Button::CANCEL:
      return "A2_CANCEL";
    case A2Button::CC_PLUS:
      return "A2_CC_PLUS";
    case A2Button::CC_MINUS_MODE:
      return "A2_CC_MINUS_MODE";
    case A2Button::PHONE:
      return "A2_PHONE";
    case A2Button::ASSISTANT:
      return "A2_ASSISTANT";
    case A2Button::NONE:
      return "";  // Return empty string for no button pressed
    default:
      return "";
  }
}

// Confirms readings with a delay between
bool confirmADCReading(int pin, int firstReading) {
  delayMicroseconds(100);
  int secondReading = analogRead(pin);

  // Allow for small variations in readings (±50)
  return abs(secondReading - firstReading) <= 50;
}

void checkA0Buttons() {
  int reading = analogRead(A0);
  A0Button currentState = lastA0State;

  if (reading <= THRESHOLD_330) {
    if (lastA0State != A0Button::NONE && confirmADCReading(A0, reading)) {
      currentState = A0Button::NONE;
    }
  } else if (lastA0State == A0Button::NONE) {
    if (reading > THRESHOLD_4095 && confirmADCReading(A0, reading))
      currentState = A0Button::OK;
    else if (reading > THRESHOLD_2300 && confirmADCReading(A0, reading))
      currentState = A0Button::RETURN;
    else if (reading > THRESHOLD_1450 && confirmADCReading(A0, reading))
      currentState = A0Button::NEXT_SONG;
    else if (reading > THRESHOLD_1100 && confirmADCReading(A0, reading))
      currentState = A0Button::PREV_SONG;
    else if (reading > THRESHOLD_720 && confirmADCReading(A0, reading))
      currentState = A0Button::VOL_UP;
    else if (reading > THRESHOLD_330 && confirmADCReading(A0, reading))
      currentState = A0Button::VOL_DOWN;
  }

  if (currentState != lastA0State) {
    lastA0State = currentState;
  }
}

void checkA1Buttons() {
  int reading = analogRead(A1);
  A1Button currentState = lastA1State;

  if (reading <= THRESHOLD_330) {
    if (lastA1State != A1Button::NONE && confirmADCReading(A1, reading)) {
      currentState = A1Button::NONE;
    }
  } else if (lastA1State == A1Button::NONE) {
    if (reading > THRESHOLD_4095 && confirmADCReading(A1, reading))
      currentState = A1Button::RADAR;
    else if (reading > THRESHOLD_2600 && confirmADCReading(A1, reading))
      currentState = A1Button::LANE_ASSIST;
    else if (reading > THRESHOLD_1450 && confirmADCReading(A1, reading))
      currentState = A1Button::LEFT_ARROW;
    else if (reading > THRESHOLD_1100 && confirmADCReading(A1, reading))
      currentState = A1Button::UP_ARROW;
    else if (reading > THRESHOLD_720 && confirmADCReading(A1, reading))
      currentState = A1Button::DOWN_ARROW;
    else if (reading > THRESHOLD_330 && confirmADCReading(A1, reading))
      currentState = A1Button::RIGHT_ARROW;
  }

  if (currentState != lastA1State) {
    lastA1State = currentState;
  }
}

void checkA2Buttons() {
  int reading = analogRead(A2);
  A2Button currentState = lastA2State;

  if (reading <= THRESHOLD_250) {
    if (lastA2State != A2Button::NONE && confirmADCReading(A2, reading)) {
      currentState = A2Button::NONE;
    }
  } else if (lastA2State == A2Button::NONE) {
    if (reading > THRESHOLD_4095 && confirmADCReading(A2, reading))
      currentState = A2Button::CRUISE_CONTROL;
    else if (reading > THRESHOLD_2600 && confirmADCReading(A2, reading))
      currentState = A2Button::CANCEL;
    else if (reading > THRESHOLD_1500 && confirmADCReading(A2, reading))
      currentState = A2Button::CC_PLUS;
    else if (reading > THRESHOLD_750 && confirmADCReading(A2, reading))
      currentState = A2Button::CC_MINUS_MODE;
    else if (reading > THRESHOLD_500 && confirmADCReading(A2, reading))
      currentState = A2Button::PHONE;
    else if (reading > THRESHOLD_250 && confirmADCReading(A2, reading))
      currentState = A2Button::ASSISTANT;
  }

  if (currentState != lastA2State) {
    lastA2State = currentState;
  }
}

String getDigitalInputString(int pin) {
  switch (pin) {
    case D10:
      return "D10_PRESSED";
    case D9:
      return "D9_PRESSED";
    case D8:
      return "D8_PRESSED";
    default:
      return "";
  }
}

String getDigitalInputReleasedString(int pin) {
  switch (pin) {
    case D10:
      return "D10_RELEASED";
    case D9:
      return "D9_RELEASED";
    case D8:
      return "D8_RELEASED";
    default:
      return "";
  }
}

// Function to read and process digital inputs
void checkDigitalInputs() {
  // Read current states
  bool currentD10State = digitalRead(D10);
  bool currentD9State = digitalRead(D9);
  bool currentD8State = digitalRead(D8);

  // Check D10
  if (currentD10State != lastD10State) {
    lastD10State = currentD10State;
  }

  // Check D9
  if (currentD9State != lastD9State) {
    lastD9State = currentD9State;
  }

  // Check D8
  if (currentD8State != lastD8State) {
    lastD8State = currentD8State;
  }
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

String handleButtons() {
  checkA0Buttons();
  checkA1Buttons();
  checkA2Buttons();
  checkDigitalInputs();

  String result = getA0ButtonString(lastA0State);
  if (result.length() == 0) result = getA1ButtonString(lastA1State);
  if (result.length() == 0) result = getA2ButtonString(lastA2State);

  if (lastD10State == LOW) {
    if (result.length() == 0) {
      result = getDigitalInputString(D10);
    }
  } else if (!lastD10State && lastD10State) {
    if (result.length() == 0) {
      result = getDigitalInputReleasedString(D10);
    }
  }

  if (lastD9State == LOW) {
    if (result.length() == 0) {
      result = getDigitalInputString(D9);
    }
  } else if (!lastD9State && lastD9State) {
    if (result.length() == 0) {
      result = getDigitalInputReleasedString(D9);
    }
  }

  if (lastD8State == LOW) {
    if (result.length() == 0) {
      result = getDigitalInputString(D8);
    }
  } else if (!lastD8State && lastD8State) {
    if (result.length() == 0) {
      result = getDigitalInputReleasedString(D8);
    }
  }

  return result;
}