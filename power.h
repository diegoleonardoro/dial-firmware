// power.h
// Idle timer and system-off sleep. In system-off the nRF52840 draws a few microamps;
// pressing any button resets the chip, so it wakes up exactly like a fresh power-on.
#pragma once
#include <Arduino.h>

void powerBegin();
void powerNoteActivity(uint32_t now);   // call whenever a dial or button is used
void powerService(uint32_t now);        // sleeps once IDLE_TIMEOUT_MS has passed with no activity
void powerSleepNow();
