// encoders.h
// Two quadrature rotary encoders (24 detents, 24 pulses per revolution) read with interrupts.
#pragma once
#include <Arduino.h>

void encodersBegin();

// Returns how many detents the dial moved since the last call (signed, clamped to -127..127).
// Anything beyond the clamp stays queued for the next call, so no detent is ever lost.
int8_t encodersTakeFront();
int8_t encodersTakeRear();

// Releases the encoder pins before system-off sleep so their pull-ups cannot drain the battery.
void encodersShutdown();
