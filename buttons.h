// buttons.h
// Debounced switches: two encoder pushes, the two-stage shutter, and the mode button.
#pragma once
#include <Arduino.h>
#include "protocol.h"

void buttonsBegin(EventSink sink);

// Call every loop. Sends button events to the sink. Returns true if any switch changed.
bool buttonsUpdate();

// Pins that should wake the controller from system-off sleep.
extern const uint8_t kWakePins[];
extern const uint8_t kWakePinCount;
