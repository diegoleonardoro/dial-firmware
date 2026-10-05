// status_led.cpp
#include "status_led.h"
#include "dial_config.h"
#include "protocol.h"
#include "ble_link.h"
#include "battery.h"

namespace {

enum Color : uint8_t { OFF, RED, GREEN, BLUE, WHITE };

Color shown = WHITE;   // force the first write

void writePin(uint8_t pin, bool on) {
  digitalWrite(pin, (on ^ LED_ACTIVE_LOW) ? HIGH : LOW);
}

void show(Color c) {
  if (c == shown) return;
  shown = c;
  writePin(LED_RED,   c == RED   || c == WHITE);
  writePin(LED_GREEN, c == GREEN || c == WHITE);
  writePin(LED_BLUE,  c == BLUE  || c == WHITE);
}

}  // namespace

void statusLedBegin() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  show(OFF);
}

void statusLedService(uint32_t now) {
  switch (bleLedOverride()) {
    case LED_OVR_RED:   show(RED);   return;
    case LED_OVR_GREEN: show(GREEN); return;
    case LED_OVR_BLUE:  show(BLUE);  return;
    case LED_OVR_WHITE: show(WHITE); return;
    case LED_OVR_OFF:   show(OFF);   return;
    default: break;   // LED_OVR_AUTO
  }

  Color statusColor = batteryIsLow() ? RED : BLUE;
  if (bleConnected()) {
    show(statusColor);
  } else {
    bool on = (now % LED_BLINK_PERIOD_MS) < LED_BLINK_ON_MS;
    show(on ? statusColor : OFF);
  }
}

void statusLedOff() {
  show(OFF);
}
