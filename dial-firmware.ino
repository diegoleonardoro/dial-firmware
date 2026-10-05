// dial-firmware.ino
// iPhone Camera Dial Controller, firmware 0.1.0
// Board: Seeed XIAO nRF52840  (Tools > Board > Seeed nRF52 Boards > Seeed XIAO nRF52840)
// Libraries: Bluefruit (comes with the board package), Bounce2 (Library Manager)
//
// What this sketch does, every loop (about 1000 times a second):
//   1. buttons:  read the 5 switches, emit press/release and mode gestures
//   2. dials:    every 15 ms, take the detents the interrupt counted and emit one event per dial
//   3. ble:      send queued events as 4-byte notifications (see PROTOCOL.md)
//   4. battery:  every 60 s, measure and update the Battery Service
//   5. led:      blue blink advertising / solid connected / red if battery low
//   6. power:    after 10 min with no input, system-off sleep; any button wakes it
//
// Files: dial_config.h (all settings), protocol.h (event codes), encoders, buttons,
//        ble_link, battery, status_led, power.

#include "dial_config.h"
#include "protocol.h"
#include "encoders.h"
#include "buttons.h"
#include "ble_link.h"
#include "battery.h"
#include "status_led.h"
#include "power.h"

static uint32_t lastDialReportMs = 0;
static bool wasConnected = false;
static int32_t frontTotal = 0, rearTotal = 0;   // detents since boot, for the 240-detent test

// Every input ends up here. It counts as activity (resets the sleep timer) and goes to BLE.
static void onInputEvent(uint8_t event, int8_t value, uint8_t flags) {
  bleQueueEvent(event, value, flags);
#if DEBUG_SERIAL
  if (!bleSubscribed()) {
    DBG("[in] event %u value %d flags %u (not sent: no phone subscribed)\n", event, value, flags);
  }
#endif
}

void setup() {
  // FIRST, before anything else: the board package boots with VBAT_ENABLE (P0.14) HIGH, and
  // Seeed warns that HIGH can damage the battery-sense pin while charging. Drive it LOW now.
  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, LOW);

#if DEBUG_SERIAL
  Serial.begin(115200);
#if DEBUG_WAIT_FOR_SERIAL_MS > 0
  uint32_t start = millis();
  while (!Serial && (millis() - start) < DEBUG_WAIT_FOR_SERIAL_MS) delay(10);
#endif
#endif
  DBG("\n=== Dial Controller firmware %s ===\n", FW_VERSION);

  statusLedBegin();
  batteryBegin();
  powerBegin();
  encodersBegin();
  buttonsBegin(onInputEvent);
  bleBegin();
  batteryService(millis(), true);
}

void loop() {
  uint32_t now = millis();

  if (buttonsUpdate()) powerNoteActivity(now);

  if (now - lastDialReportMs >= DIAL_REPORT_INTERVAL_MS) {
    lastDialReportMs = now;
    // If the previous turn of a dial hasn't gone out yet (radio busy), leave new detents in
    // the encoder counter; they are added up and sent as one larger value next time.
    if (!bleHasPending(EVT_FRONT_DIAL)) {
      int8_t front = encodersTakeFront();
      if (front != 0) {
        frontTotal += front;
        DBG("[dial] front %+d  total %ld\n", front, (long)frontTotal);
        onInputEvent(EVT_FRONT_DIAL, front, 0);
        powerNoteActivity(now);
      }
    }
    if (!bleHasPending(EVT_REAR_DIAL)) {
      int8_t rear = encodersTakeRear();
      if (rear != 0) {
        rearTotal += rear;
        DBG("[dial] rear  %+d  total %ld\n", rear, (long)rearTotal);
        onInputEvent(EVT_REAR_DIAL, rear, 0);
        powerNoteActivity(now);
      }
    }
  }

  bleService(now);

  // Fresh battery reading on every new connection, then every 60 s.
  bool isConnected = bleConnected();
  batteryService(now, isConnected && !wasConnected);
  wasConnected = isConnected;

  statusLedService(now);
  powerService(now);

  // Sleep the CPU until the next millisecond instead of spinning; saves battery.
  delay(1);
}
