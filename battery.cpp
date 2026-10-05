// battery.cpp
//
// The XIAO nRF52840 has a resistor divider from the battery: BAT -> 1 MΩ -> P0.31 -> 510 kΩ -> P0.14.
// When P0.14 (VBAT_ENABLE) is driven LOW, P0.31 (PIN_VBAT) sees BAT × 510 / 1510.
//
// Important: Seeed's board package sets VBAT_ENABLE HIGH at boot, but Seeed's own wiki warns that
// with P0.14 HIGH, P0.31 can be pushed past its 3.6 V limit while charging and be damaged.
// So setup() drives it LOW as its very first line and it stays LOW forever (costs ~3 µA).
//
// Verify with your multimeter once the battery is soldered on: the voltage printed in the
// Serial Monitor should be within ~0.05 V of what the meter reads across the BAT pads.
// If it is consistently off, adjust VBAT_CALIBRATION.
//
// With no battery attached (USB only), the charger chip makes BAT read roughly 4.2 V or wander;
// the percentage means nothing until a battery is connected.
//
#include "battery.h"
#include "dial_config.h"
#include "ble_link.h"

namespace {

const float kDividerRatio   = (1000.0f + 510.0f) / 510.0f;   // ≈ 2.961
const float kAdcReferenceV  = 3.0f;                          // AR_INTERNAL_3_0
const float kAdcCounts      = 4096.0f;                       // 12-bit
const float VBAT_CALIBRATION = 1.000f;                       // multiply to match your multimeter

// Resting LiPo voltage -> percent. Rough, but good enough for a low-battery LED.
struct Point { float v; uint8_t pct; };
const Point kCurve[] = {
  {4.20f, 100}, {4.10f, 90}, {4.00f, 80}, {3.92f, 70}, {3.86f, 60}, {3.82f, 50},
  {3.79f, 40},  {3.77f, 30}, {3.74f, 20}, {3.71f, 15}, {3.68f, 10}, {3.60f, 5}, {3.30f, 0},
};
const size_t kCurveLen = sizeof(kCurve) / sizeof(kCurve[0]);

float    lastVolts = 0.0f;
uint8_t  lastPercent = 100;
bool     low = false;
uint32_t lastReportMs = 0;
bool     everReported = false;

uint8_t voltsToPercent(float v) {
  if (v >= kCurve[0].v) return 100;
  for (size_t i = 1; i < kCurveLen; i++) {
    if (v >= kCurve[i].v) {
      const Point& hi = kCurve[i - 1];
      const Point& lo = kCurve[i];
      float t = (v - lo.v) / (hi.v - lo.v);
      return (uint8_t)(lo.pct + t * (hi.pct - lo.pct) + 0.5f);
    }
  }
  return 0;
}

float measure() {
  uint32_t sum = 0;
  const int kSamples = 8;
  for (int i = 0; i < kSamples; i++) sum += analogRead(PIN_VBAT);
  float adcVolts = ((float)sum / kSamples) * kAdcReferenceV / kAdcCounts;
  return adcVolts * kDividerRatio * VBAT_CALIBRATION;
}

}  // namespace

void batteryBegin() {
  // setup() already drove this LOW as its very first action; repeated here so this file
  // is safe on its own. Never set it HIGH (see top of file).
  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, LOW);

  analogReference(AR_INTERNAL_3_0); // 0..3.0 V input range
  analogReadResolution(12);
  analogSampleTime(40);             // the divider is high-impedance; 40 µs lets the ADC settle
  delay(1);
}

void batteryService(uint32_t now, bool force) {
  if (!force && everReported && (now - lastReportMs) < BATTERY_REPORT_INTERVAL_MS) return;
  lastReportMs = now;
  everReported = true;

  lastVolts = measure();
  lastPercent = voltsToPercent(lastVolts);

  if (!low && lastPercent < BATTERY_LOW_PERCENT) low = true;
  else if (low && lastPercent >= BATTERY_LOW_CLEAR_PERCENT) low = false;

  bleSetBatteryLevel(lastPercent);
  DBG("[bat] %.3f V  %u%%%s\n", lastVolts, lastPercent, low ? "  LOW" : "");
}

float   batteryVolts()   { return lastVolts; }
uint8_t batteryPercent() { return lastPercent; }
bool    batteryIsLow()   { return low; }
