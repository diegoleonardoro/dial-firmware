// battery.h
// LiPo voltage through the XIAO's on-board divider, reported via the Battery Service.
#pragma once
#include <Arduino.h>

void batteryBegin();

// Call every loop; measures and reports every BATTERY_REPORT_INTERVAL_MS. Pass force=true to measure now.
void batteryService(uint32_t now, bool force = false);

float   batteryVolts();      // last measurement
uint8_t batteryPercent();    // last measurement, 0..100
bool    batteryIsLow();      // below BATTERY_LOW_PERCENT (with hysteresis)
