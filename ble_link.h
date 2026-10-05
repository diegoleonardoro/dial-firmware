// ble_link.h
// Everything Bluetooth: GATT services, advertising, connection parameters, the outgoing event queue.
#pragma once
#include <Arduino.h>
#include "protocol.h"

void bleBegin();

// Call every loop: sends queued events, requests connection parameters after connecting.
// Never blocks.
void bleService(uint32_t now);

// Queue one 4-byte event. Dropped (returns false) when no phone is subscribed.
bool bleQueueEvent(uint8_t event, int8_t value, uint8_t flags);

// True if an event with this code is queued but not yet sent. Used to keep dial turns in the
// encoder counter (where they add up) instead of piling up separate packets in the queue.
bool bleHasPending(uint8_t event);

// True when connected AND the phone has enabled notifications on Input Event.
bool bleSubscribed();
bool bleConnected();

// Battery Service: store the level (readable any time) and notify it if the phone subscribed.
void bleSetBatteryLevel(uint8_t percent);

// Current LED override written by the phone (LED_OVR_AUTO when none).
uint8_t bleLedOverride();

// Before sleep: disconnect cleanly and stop advertising.
void bleShutdown();
