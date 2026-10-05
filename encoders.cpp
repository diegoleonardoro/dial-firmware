// encoders.cpp
//
// How a mechanical encoder works: two switches, A and B, open and close a quarter-cycle apart.
// With pull-ups, each switch reads 1 when open and 0 when closed, so the pair (A,B) walks
// through the Gray-code cycle 11 -> 01 -> 00 -> 10 -> 11 in one direction (A leads) and the
// reverse in the other (B leads). These encoders do one full cycle per detent (24 detents = 24 pulses),
// and they rest at the same state at every click (normally 11, both open).
//
// The decoder below:
//   1. On every edge of A or B, looks up the (previous, current) pair in a 16-entry table
//      that says +1 (a quarter step forward), -1 (a quarter step back) or 0 (no move or an
//      impossible jump, which is what contact bounce and missed edges look like).
//   2. Adds that to a quarter-step counter.
//   3. When the pins come back to the rest state, rounds the counter to whole detents and
//      clears it. Bounce adds +1 then -1, which cancels, so it never produces a false detent.
//
#include "encoders.h"
#include "dial_config.h"

namespace {

// Index = (previous AB << 2) | current AB. Value = quarter steps.
// Sign convention: A changing before B = positive. Most encoder datasheets (including the
// Bourns PEC11R) define that as clockwise. If yours counts backwards, use the *_DIAL_INVERT flags.
const int8_t kQuarterStep[16] = {
   0, -1, +1,  0,   // prev 00 -> 00, 01, 10, 11
  +1,  0,  0, -1,   // prev 01 -> 00, 01, 10, 11
  -1,  0,  0, +1,   // prev 10 -> 00, 01, 10, 11
   0, +1, -1,  0,   // prev 11 -> 00, 01, 10, 11
};

struct Encoder {
  uint8_t pinA;
  uint8_t pinB;
  bool invert;
  uint8_t restState;            // AB value at a detent, learned at boot
  volatile uint8_t prevState;   // last AB value seen by the interrupt
  volatile int16_t quarters;    // quarter steps since the last detent
  volatile int32_t detents;     // whole detents not yet taken by the main loop
};

Encoder front = { PIN_FRONT_ENC_A, PIN_FRONT_ENC_B, FRONT_DIAL_INVERT, 0b11, 0b11, 0, 0 };
Encoder rear  = { PIN_REAR_ENC_A,  PIN_REAR_ENC_B,  REAR_DIAL_INVERT,  0b11, 0b11, 0, 0 };

inline uint8_t readAB(const Encoder& e) {
  return (uint8_t)((digitalRead(e.pinA) << 1) | digitalRead(e.pinB));
}

// Runs inside the GPIO interrupt. Keep it short: no Serial, no BLE calls.
// It re-reads the pins until they stop changing, because the core clears the interrupt flag
// only after this returns: an edge arriving while we run would otherwise go unnoticed.
inline void handleEdge(Encoder& e) {
  for (uint8_t pass = 0; pass < 4; pass++) {
    uint8_t now = readAB(e);
    uint8_t prev = e.prevState;
    if (now == prev) return;

    int16_t q = e.quarters + kQuarterStep[(prev << 2) | now];
    e.prevState = now;

    if (now == e.restState) {
      // Back at a detent. +4 is a clean detent; +2/+3 means an edge was missed but the knob
      // clearly moved one click; +6 or more means a whole rest state was missed in a fast spin.
      int32_t steps = (q >= 0 ? q + 2 : q - 2) / 4;
      q = 0;
      if (steps != 0) {
        if (e.invert) steps = -steps;
        __atomic_fetch_add(&e.detents, steps, __ATOMIC_RELAXED);
      }
    }
    e.quarters = q;
  }
}

void frontIsr() { handleEdge(front); }
void rearIsr()  { handleEdge(rear); }

void setupEncoder(Encoder& e, void (*isr)()) {
  pinMode(e.pinA, INPUT_PULLUP);
  pinMode(e.pinB, INPUT_PULLUP);
  delay(2);  // let the pull-ups charge the wiring before the first read

  uint8_t s = readAB(e);
  // Full-cycle encoders rest at 11 or 00. If we booted mid-turn (01 or 10), assume 11.
  e.restState = (s == 0b00) ? 0b00 : 0b11;
  e.prevState = s;
  e.quarters = 0;
  e.detents = 0;

  attachInterrupt(digitalPinToInterrupt(e.pinA), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(e.pinB), isr, CHANGE);
}

int8_t take(Encoder& e) {
  int32_t pending = __atomic_load_n(&e.detents, __ATOMIC_RELAXED);
  if (pending == 0) return 0;
  if (pending > 127) pending = 127;
  if (pending < -127) pending = -127;
  // Subtract only what we report; anything the interrupt adds meanwhile is kept.
  __atomic_fetch_sub(&e.detents, pending, __ATOMIC_RELAXED);
  return (int8_t)pending;
}

void releasePins(Encoder& e) {
  detachInterrupt(digitalPinToInterrupt(e.pinA));
  detachInterrupt(digitalPinToInterrupt(e.pinB));
  // Input buffer disconnected, no pull resistor: draws nothing even if the contacts are closed.
  nrf_gpio_cfg_default(g_ADigitalPinMap[e.pinA]);
  nrf_gpio_cfg_default(g_ADigitalPinMap[e.pinB]);
}

}  // namespace

void encodersBegin() {
  setupEncoder(front, frontIsr);
  setupEncoder(rear, rearIsr);
  DBG("[enc] rest state front=%d%d rear=%d%d\n",
      (front.restState >> 1) & 1, front.restState & 1,
      (rear.restState >> 1) & 1, rear.restState & 1);
}

int8_t encodersTakeFront() { return take(front); }
int8_t encodersTakeRear()  { return take(rear); }

void encodersShutdown() {
  releasePins(front);
  releasePins(rear);
}
