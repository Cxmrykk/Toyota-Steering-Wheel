#include <Arduino.h>

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

// Function to read and process A0 buttons
void checkA0Buttons()
{
  int reading = analogRead(A0);

  if (reading > THRESHOLD_4095)
  {
    Serial.println("A0: OK");
  }
  else if (reading > THRESHOLD_2300)
  {
    Serial.println("A0: Return");
  }
  else if (reading > THRESHOLD_1450)
  {
    Serial.println("A0: Next Song");
  }
  else if (reading > THRESHOLD_1100)
  {
    Serial.println("A0: Prev Song");
  }
  else if (reading > THRESHOLD_720)
  {
    Serial.println("A0: Vol Up");
  }
  else if (reading > THRESHOLD_330)
  {
    Serial.println("A0: Vol Down");
  }
}

// Function to read and process A1 buttons
void checkA1Buttons()
{
  int reading = analogRead(A1);

  if (reading > THRESHOLD_4095)
  {
    Serial.println("A1: Radar");
  }
  else if (reading > THRESHOLD_2600)
  {
    Serial.println("A1: Lane Assist");
  }
  else if (reading > THRESHOLD_1450)
  {
    Serial.println("A1: Left Arrow");
  }
  else if (reading > THRESHOLD_1100)
  {
    Serial.println("A1: Up Arrow");
  }
  else if (reading > THRESHOLD_720)
  {
    Serial.println("A1: Down Arrow");
  }
  else if (reading > THRESHOLD_330)
  {
    Serial.println("A1: Right Arrow");
  }
}

// Function to read and process A2 buttons
void checkA2Buttons()
{
  int reading = analogRead(A2);

  if (reading > THRESHOLD_4095)
  {
    Serial.println("A2: Cruise Control");
  }
  else if (reading > THRESHOLD_2600)
  {
    Serial.println("A2: Cancel");
  }
  else if (reading > THRESHOLD_1500)
  {
    Serial.println("A2: CC Plus");
  }
  else if (reading > THRESHOLD_750)
  {
    Serial.println("A2: CC Minus and Mode");
  }
  else if (reading > THRESHOLD_500)
  {
    Serial.println("A2: Phone");
  }
  else if (reading > THRESHOLD_250)
  {
    Serial.println("A2: Assistant");
  }
}

// Function to check digital inputs
void checkDigitalInputs()
{
  if (digitalRead(D10) == LOW)
  {
    Serial.println("D10: Paddle left");
  }
  if (digitalRead(D9) == LOW)
  {
    Serial.println("D9: Paddle right");
  }
  if (digitalRead(D8) == LOW)
  {
    Serial.println("D8: Horn");
  }
}

void setup()
{
  Serial.begin(115200);

  // Configure ADC resolution to 12 bits
  analogReadResolution(12);

  // Configure analog pins as inputs
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  // Configure digital pins as inputs with pull-up resistors
  pinMode(D10, INPUT_PULLUP);
  pinMode(D9, INPUT_PULLUP);
  pinMode(D8, INPUT_PULLUP);
}

void loop()
{
  // Check all buttons
  checkA0Buttons();
  checkA1Buttons();
  checkA2Buttons();
  checkDigitalInputs();

  // Add a short delay
  delay(1);
}