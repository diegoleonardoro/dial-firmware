// dial_config.h
// Every number you might want to change lives here: pins, timings, UUIDs, version.
// Board: Seeed XIAO nRF52840 with the "Seeed nRF52 Boards" package (NOT the mbed one).
#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Version. Bump this whenever you tag a release in git.
// ---------------------------------------------------------------------------
#define FW_VERSION "0.1.0"

// ---------------------------------------------------------------------------
// BLE UUIDs. Must match PROTOCOL.md and the iPhone app exactly.
// Written in normal (big-endian) form; the Bluefruit library reverses them itself.
// ---------------------------------------------------------------------------
#define UUID_DIAL_SERVICE   "188D0001-8E0D-4A9D-9B04-830DCBEB93B8"
#define UUID_INPUT_EVENT    "188D0002-8E0D-4A9D-9B04-830DCBEB93B8"
#define UUID_FEEDBACK       "188D0003-8E0D-4A9D-9B04-830DCBEB93B8"

#define DIS_MANUFACTURER    "DIY"
#define DIS_MODEL           "Dial Controller"

// ---------------------------------------------------------------------------
// Pins (from the guide's pin table). D0..D10 are the labels printed on the XIAO.
// Every input is wired between the pin and GND; the firmware turns on internal pull-ups.
// ---------------------------------------------------------------------------
#define PIN_FRONT_ENC_A     D0
#define PIN_FRONT_ENC_B     D1
#define PIN_FRONT_PUSH      D2
#define PIN_REAR_ENC_A      D3
#define PIN_REAR_ENC_B      D4
#define PIN_REAR_PUSH       D5
#define PIN_SHUTTER_HALF    D6
#define PIN_SHUTTER_FULL    D7
#define PIN_MODE            D8

// Onboard RGB LED. On the XIAO nRF52840 it is common-anode: LOW = on.
// (Seeed's variant.h says LED_STATE_ON is 1; the board actually lights on LOW.
//  If your LED behaves inverted, set this to false.)
#define LED_ACTIVE_LOW      true

// Flip a dial's direction if clockwise comes out negative on your breadboard.
#define FRONT_DIAL_INVERT   false
#define REAR_DIAL_INVERT    false

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------
#define DIAL_REPORT_INTERVAL_MS   15      // coalesce dial turns; at most one notify per dial per 15 ms
#define DEBOUNCE_MS               5       // switch must be stable this long to count
#define MODE_SHORT_MAX_MS         400     // short press: released before this
#define MODE_LONG_MIN_MS          800     // long press: fires once held this long

#define BATTERY_REPORT_INTERVAL_MS 60000UL // Battery Service update period
#define BATTERY_LOW_PERCENT        15      // LED turns red below this
#define BATTERY_LOW_CLEAR_PERCENT  20      // ...and back to blue above this (hysteresis)

// Idle time before system-off sleep. 10 minutes per the spec.
// For testing the sleep/wake checklist item, temporarily set this to 30000UL (30 s).
// For the overnight 8-hour battery test, set it to (24UL * 60UL * 60UL * 1000UL) so the
// controller does not fall asleep after 10 minutes and end the test early.
#define IDLE_TIMEOUT_MS           (10UL * 60UL * 1000UL)

// Status LED blink while advertising
#define LED_BLINK_PERIOD_MS       1000
#define LED_BLINK_ON_MS           100

// ---------------------------------------------------------------------------
// BLE connection parameters (Apple Accessory Design Guidelines compliant).
// Units: interval 1.25 ms, timeout 10 ms.
// ---------------------------------------------------------------------------
#define CONN_INTERVAL_MIN_UNITS   12      // 12 x 1.25 = 15 ms
#define CONN_INTERVAL_MAX_UNITS   24      // 24 x 1.25 = 30 ms
#define CONN_PERIPHERAL_LATENCY   0
#define CONN_SUP_TIMEOUT_UNITS    400     // 400 x 10 = 4 s
#define CONN_PARAM_REQUEST_DELAY_MS 1000  // wait this long after connecting before asking

// Advertising: 20 ms for 30 s, then 152.5 ms (Apple's recommended values). Units 0.625 ms.
#define ADV_FAST_INTERVAL_UNITS   32      // 20 ms
#define ADV_SLOW_INTERVAL_UNITS   244     // 152.5 ms
#define ADV_FAST_TIMEOUT_S        30

#define BLE_TX_POWER_DBM          4       // 0 is plenty on a desk; 4 helps with a hand around it

// ---------------------------------------------------------------------------
// Debug output over USB serial (Arduino Serial Monitor, 115200 baud).
// Set DEBUG_WAIT_FOR_SERIAL_MS to 3000 if you want to see the boot messages;
// leave it at 0 for normal use so the controller boots instantly on battery.
// ---------------------------------------------------------------------------
// Turn DEBUG_SERIAL off (0) for the 0.2.0 release build: it is for development only.
#define DEBUG_SERIAL              1
#define DEBUG_WAIT_FOR_SERIAL_MS  0

#if DEBUG_SERIAL
  #define DBG(...)    Serial.printf(__VA_ARGS__)
#else
  #define DBG(...)    do {} while (0)
#endif
