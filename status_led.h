// status_led.h
// The XIAO's onboard RGB LED as the status light.
//   blue blink  = advertising (waiting for the phone)
//   solid blue  = connected
//   red instead of blue = battery low (same blink/solid pattern, so you still see the connection state)
//   the phone can override the colour through the Feedback characteristic
#pragma once
#include <Arduino.h>

void statusLedBegin();
void statusLedService(uint32_t now);
void statusLedOff();   // before sleep
