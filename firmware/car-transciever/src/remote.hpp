#pragma once

#include <Arduino.h>
#include <map>

// PWM Configuration
#define PIN_AUX_LATCH D2

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
    {ButtonID::VOLUME_UP, 0},
    {ButtonID::VOLUME_DOWN, 127 + 16},

    // Media controls on AUX_1
    {ButtonID::NEXT_SONG, 127 + 32},
    {ButtonID::PREV_SONG, 127 + 48},

    // Phone controls on AUX_2
    {ButtonID::PHONE, 127 + 64},
    {ButtonID::RETURN, 127 + 80},

    // Navigation controls on AUX_2
    {ButtonID::ASSISTANT, 127 + 96},
    {ButtonID::MODE, 127 + 112},
};

// Function to set PWM output for a button
inline void setRemoteState(ButtonID button, bool isPressed) {
  auto it = BUTTON_SPI_MAP.find(button);
  if (it != BUTTON_SPI_MAP.end()) {
    Serial.println(static_cast<int>(button));

    // Start the SPI transaction (20 Mhz)
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));

    // Pull latch pin low to start transfer
    digitalWrite(PIN_AUX_LATCH, LOW);

    // Transfer data for each register
    SPI.transfer(isPressed ? it->second : 255);

    // Pull latch pin high to update outputs
    digitalWrite(PIN_AUX_LATCH, HIGH);

    // End the SPI transaction
    SPI.endTransaction();
  }
}
