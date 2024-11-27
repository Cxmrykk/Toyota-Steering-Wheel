#include <Arduino.h>

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

// Function to print A0 button state
void printA0Button(A0Button state) {
  switch (state) {
    case A0Button::OK:
      Serial.println("A0: OK");
      break;
    case A0Button::RETURN:
      Serial.println("A0: Return");
      break;
    case A0Button::NEXT_SONG:
      Serial.println("A0: Next Song");
      break;
    case A0Button::PREV_SONG:
      Serial.println("A0: Prev Song");
      break;
    case A0Button::VOL_UP:
      Serial.println("A0: Vol Up");
      break;
    case A0Button::VOL_DOWN:
      Serial.println("A0: Vol Down");
      break;
    case A0Button::NONE:
      Serial.println("A0: Released");
      break;
  }
}

// Function to print A1 button state
void printA1Button(A1Button state) {
  switch (state) {
    case A1Button::RADAR:
      Serial.println("A1: Radar");
      break;
    case A1Button::LANE_ASSIST:
      Serial.println("A1: Lane Assist");
      break;
    case A1Button::LEFT_ARROW:
      Serial.println("A1: Left Arrow");
      break;
    case A1Button::UP_ARROW:
      Serial.println("A1: Up Arrow");
      break;
    case A1Button::DOWN_ARROW:
      Serial.println("A1: Down Arrow");
      break;
    case A1Button::RIGHT_ARROW:
      Serial.println("A1: Right Arrow");
      break;
    case A1Button::NONE:
      Serial.println("A1: Released");
      break;
  }
}

// Function to print A2 button state
void printA2Button(A2Button state) {
  switch (state) {
    case A2Button::CRUISE_CONTROL:
      Serial.println("A2: Cruise Control");
      break;
    case A2Button::CANCEL:
      Serial.println("A2: Cancel");
      break;
    case A2Button::CC_PLUS:
      Serial.println("A2: CC Plus");
      break;
    case A2Button::CC_MINUS_MODE:
      Serial.println("A2: CC Minus and Mode");
      break;
    case A2Button::PHONE:
      Serial.println("A2: Phone");
      break;
    case A2Button::ASSISTANT:
      Serial.println("A2: Assistant");
      break;
    case A2Button::NONE:
      Serial.println("A2: Released");
      break;
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
    printA0Button(currentState);
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
    printA1Button(currentState);
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
    printA2Button(currentState);
    lastA2State = currentState;
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
    if (currentD10State == LOW) {
      Serial.println("D10: Paddle left");
    } else {
      Serial.println("D10: Released");
    }
    lastD10State = currentD10State;
  }

  // Check D9
  if (currentD9State != lastD9State) {
    if (currentD9State == LOW) {
      Serial.println("D9: Paddle right");
    } else {
      Serial.println("D9: Released");
    }
    lastD9State = currentD9State;
  }

  // Check D8
  if (currentD8State != lastD8State) {
    if (currentD8State == LOW) {
      Serial.println("D8: Horn");
    } else {
      Serial.println("D8: Released");
    }
    lastD8State = currentD8State;
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  pinMode(D10, INPUT_PULLUP);
  pinMode(D9, INPUT_PULLUP);
  pinMode(D8, INPUT_PULLUP);
}

void loop() {
  checkA0Buttons();
  checkA1Buttons();
  checkA2Buttons();
  checkDigitalInputs();
}