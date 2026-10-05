// protocol.h
// Byte-level constants from PROTOCOL.md. If you edit this, edit PROTOCOL.md too.
#pragma once
#include <stdint.h>

// Input Event characteristic: 4 bytes = event, value (int8), flags, sequence
enum EventCode : uint8_t {
  EVT_FRONT_DIAL   = 0x01,
  EVT_REAR_DIAL    = 0x02,
  EVT_FRONT_PRESS  = 0x03,
  EVT_REAR_PRESS   = 0x04,
  EVT_SHUTTER_HALF = 0x05,
  EVT_SHUTTER_FULL = 0x06,
  EVT_MODE_SHORT   = 0x07,
  EVT_MODE_LONG    = 0x08,
};

// flags byte
static const uint8_t FLAG_PRESSED  = 0x01;   // bit 0 = 1 pressed, 0 released
static const uint8_t FLAG_RELEASED = 0x00;

// Feedback characteristic: 2 bytes = command, argument
static const uint8_t CMD_LED_OVERRIDE = 0x01;

enum LedOverride : uint8_t {
  LED_OVR_AUTO  = 0x00,
  LED_OVR_RED   = 0x01,
  LED_OVR_GREEN = 0x02,
  LED_OVR_BLUE  = 0x03,
  LED_OVR_WHITE = 0x04,
  LED_OVR_OFF   = 0x05,
};

// Every module reports input through one function with this shape (see dial-firmware.ino).
typedef void (*EventSink)(uint8_t event, int8_t value, uint8_t flags);
