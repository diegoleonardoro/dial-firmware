// power.cpp
#include "power.h"
#include "dial_config.h"
#include "buttons.h"
#include "encoders.h"
#include "status_led.h"
#include "ble_link.h"

namespace {
uint32_t lastActivityMs = 0;
}

void powerBegin() {
  lastActivityMs = millis();

  uint32_t reason = readResetReason();
  if (reason & POWER_RESETREAS_OFF_Msk)            DBG("[pwr] woke from sleep by a button\n");
  else if (reason & POWER_RESETREAS_VBUS_Msk)      DBG("[pwr] woke from sleep by USB power\n");
  else if (reason & POWER_RESETREAS_RESETPIN_Msk)  DBG("[pwr] reset button\n");
  else if (reason & POWER_RESETREAS_SREQ_Msk)      DBG("[pwr] software reset (e.g. after upload)\n");
  else                                             DBG("[pwr] power-on\n");
}

void powerNoteActivity(uint32_t now) {
  lastActivityMs = now;
}

void powerService(uint32_t now) {
  if (now - lastActivityMs >= IDLE_TIMEOUT_MS) {
    powerSleepNow();
  }
}

void powerSleepNow() {
  DBG("[pwr] idle %lu s, entering system-off. Press any button to wake.\n",
      (unsigned long)(IDLE_TIMEOUT_MS / 1000UL));

  bleShutdown();        // disconnect cleanly so the phone notices immediately
  statusLedOff();       // GPIO states are kept during system-off, so the LED must be off now
  encodersShutdown();   // remove encoder pull-ups so closed contacts cannot drain the battery
  delay(20);            // let the last serial message go out

  // Arm each button as a wake source: pull-up on, wake when the pin goes LOW.
  // A button that is already LOW (held down or stuck) would wake the chip instantly and
  // its pull-up would drain ~250 µA, so it is released instead and not used as a wake source.
  for (uint8_t i = 0; i < kWakePinCount; i++) {
    uint32_t pin = g_ADigitalPinMap[kWakePins[i]];
    if (digitalRead(kWakePins[i]) == LOW) {
      nrf_gpio_cfg_default(pin);
    } else {
      nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    }
  }

  // Power down. Same sequence as the core's systemOff(). Plugging in USB also wakes it.
  uint8_t sdEnabled = 0;
  (void)sd_softdevice_is_enabled(&sdEnabled);
  if (sdEnabled) {
    sd_power_system_off();
  } else {
    NRF_POWER->SYSTEMOFF = 1;
  }

  // Not reached: system-off ends with a reset.
  while (true) {}
}
