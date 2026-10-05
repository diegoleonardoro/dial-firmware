// buttons.cpp
//
// Each switch connects its pin to GND, so pressed = LOW. Bounce2 ignores the few
// milliseconds of contact chatter and reports one clean press and one clean release.
//
// The two-stage shutter is two separate switches: stage 1 (half press) closes first,
// stage 2 (full press) closes when you push harder. A full press therefore produces
//   05 pressed, 06 pressed, 06 released, 05 released
// which is exactly what a camera body sees.
//
// The mode button has two gestures:
//   short press: released before 400 ms  -> event 7, sent on release
//   long press:  held for 800 ms         -> event 8, sent the moment 800 ms is reached
//   400..800 ms: nothing (deliberate dead zone)
//
#include "buttons.h"
#include "dial_config.h"
#include <Bounce2.h>

const uint8_t kWakePins[] = { PIN_FRONT_PUSH, PIN_REAR_PUSH, PIN_SHUTTER_HALF, PIN_SHUTTER_FULL, PIN_MODE };
const uint8_t kWakePinCount = sizeof(kWakePins) / sizeof(kWakePins[0]);

namespace {

struct SimpleButton {
  uint8_t pin;
  uint8_t event;
  Bounce2::Button btn;
};

SimpleButton simple[] = {
  { PIN_FRONT_PUSH,   EVT_FRONT_PRESS,  Bounce2::Button() },
  { PIN_REAR_PUSH,    EVT_REAR_PRESS,   Bounce2::Button() },
  { PIN_SHUTTER_HALF, EVT_SHUTTER_HALF, Bounce2::Button() },
  { PIN_SHUTTER_FULL, EVT_SHUTTER_FULL, Bounce2::Button() },
};
const size_t kSimpleCount = sizeof(simple) / sizeof(simple[0]);

Bounce2::Button modeBtn;
bool modeLongFired = false;

EventSink emit = nullptr;

void setupButton(Bounce2::Button& b, uint8_t pin) {
  b.attach(pin, INPUT_PULLUP);
  b.interval(DEBOUNCE_MS);
  b.setPressedState(LOW);
}

}  // namespace

void buttonsBegin(EventSink sink) {
  emit = sink;
  for (size_t i = 0; i < kSimpleCount; i++) setupButton(simple[i].btn, simple[i].pin);
  setupButton(modeBtn, PIN_MODE);
}

bool buttonsUpdate() {
  bool activity = false;

  for (size_t i = 0; i < kSimpleCount; i++) {
    Bounce2::Button& b = simple[i].btn;
    b.update();
    if (b.pressed())  { emit(simple[i].event, 0, FLAG_PRESSED);  activity = true; }
    if (b.released()) { emit(simple[i].event, 0, FLAG_RELEASED); activity = true; }
  }

  modeBtn.update();
  if (modeBtn.pressed()) {
    modeLongFired = false;
    activity = true;
  }
  if (modeBtn.isPressed() && !modeLongFired && modeBtn.currentDuration() >= MODE_LONG_MIN_MS) {
    modeLongFired = true;
    emit(EVT_MODE_LONG, 0, 0);
    activity = true;
  }
  if (modeBtn.released()) {
    // previousDuration() = how long it was held before this release
    if (!modeLongFired && modeBtn.previousDuration() < MODE_SHORT_MAX_MS) {
      emit(EVT_MODE_SHORT, 0, 0);
    }
    activity = true;
  }

  return activity;
}
