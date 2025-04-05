#pragma once

#include <Arduino.h>
#include <map>

// PWM Configuration
#define PIN_LATCH D2

struct PWMConfig {
  uint16_t value;
};

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

// Map ButtonID to PWM configuration
const std::map<ButtonID, uint8_t> BUTTON_SPI_MAP = {
    // Volume controls on AUX_1
    {ButtonID::VOLUME_UP, 255},
    {ButtonID::VOLUME_DOWN, 255 - 1},

    // Media controls on AUX_1
    {ButtonID::NEXT_SONG, 255 - 2},
    {ButtonID::PREV_SONG, 255 - 3},

    // Phone controls on AUX_2
    {ButtonID::PHONE, 255 - 4},
    {ButtonID::RETURN, 255 - 5},

    // Navigation controls on AUX_2
    {ButtonID::ASSISTANT, 255 - 6},
    {ButtonID::MODE, 255 - 7},
};

// Function to set PWM output for a button
inline void setRemoteState(ButtonID button, bool isPressed) {
  auto it = BUTTON_SPI_MAP.find(button);
  if (it != BUTTON_SPI_MAP.end()) {
    Serial.println(static_cast<int>(button));

    // Pull latch pin low to start transfer
    digitalWrite(PIN_LATCH, LOW);

    // Transfer data for each register
    SPI.transfer(isPressed ? it->second : 0);

    // Pull latch pin high to update outputs
    digitalWrite(PIN_LATCH, HIGH);
  }
}
