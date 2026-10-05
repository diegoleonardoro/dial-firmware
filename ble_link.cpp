// ble_link.cpp
//
// GATT layout (see PROTOCOL.md):
//   Dial Controller Service   188D0001-...
//     Input Event   188D0002-...  Notify+Read, 4 bytes: event, value, flags, sequence
//     Feedback      188D0003-...  Write,       2 bytes: command, argument
//   Battery Service           0x180F
//   Device Information        0x180A
//
// Threading: Bluefruit runs its callbacks (connect, disconnect, write, raw events) in its own
// FreeRTOS task. Those callbacks only set simple flags and counters. The event queue and the
// sequence number are touched only from loop(), so they need no locking.
//
// Never-block rule: Bluefruit's notify() waits up to 100 ms when the radio's notification
// buffer is full, and loop() would stop reading buttons meanwhile. So this file counts the
// notifications the radio has not sent yet ("in flight") and only calls notify() when a
// buffer slot is guaranteed free. Anything that cannot go out now stays queued for the next loop.
//
#include "ble_link.h"
#include "dial_config.h"
#include <bluefruit.h>

namespace {

BLEService        dialService(UUID_DIAL_SERVICE);
BLECharacteristic inputChr(UUID_INPUT_EVENT);
BLECharacteristic feedbackChr(UUID_FEEDBACK);
BLEDis            dis;
BLEBas            bas;

char deviceName[12];

// Radio notification buffer size (configPrphConn below) and how many are in use.
const int32_t kHvnSlots = 8;
volatile int32_t inFlight = 0;

// Set from Bluefruit's task, read from loop()
volatile bool     connected = false;
volatile uint16_t connHandle = BLE_CONN_HANDLE_INVALID;
volatile uint32_t connectedAtMs = 0;
volatile uint32_t connectionCount = 0;
volatile uint8_t  ledOverride = LED_OVR_AUTO;
volatile int16_t  disconnectReason = -1;   // printed from loop(), not from Bluefruit's task

// Only touched by loop()
uint32_t handledConnection = 0;    // which connection loop() has already reset for
bool     paramsRequested = false;
bool     intervalLogged = false;
uint8_t  sequence = 0;
uint8_t  pendingBattery = 0xFF;    // battery level waiting to be notified (0xFF = none)

// FIFO between the inputs and the radio. Holds at most one dial event per dial
// (see bleHasPending) plus any button events.
struct Event { uint8_t b[4]; };
const uint8_t kQueueSize = 32;
Event   queue[kQueueSize];
uint8_t qHead = 0, qTail = 0;   // head = next to send, tail = next free slot

inline bool queueEmpty() { return qHead == qTail; }
inline bool queueFull()  { return (uint8_t)((qTail + 1) % kQueueSize) == qHead; }
inline void queueClear() { qHead = qTail = 0; }

// Reserve a radio buffer slot before calling notify(); give it back if notify() fails.
inline bool reserveSlot() {
  if (__atomic_add_fetch(&inFlight, 1, __ATOMIC_SEQ_CST) <= kHvnSlots) return true;
  __atomic_sub_fetch(&inFlight, 1, __ATOMIC_SEQ_CST);
  return false;
}
inline void releaseSlot() { __atomic_sub_fetch(&inFlight, 1, __ATOMIC_SEQ_CST); }

void onConnect(uint16_t conn_handle) {
  __atomic_store_n(&inFlight, 0, __ATOMIC_SEQ_CST);
  connHandle = conn_handle;
  connectedAtMs = millis();
  connectionCount = connectionCount + 1;
  connected = true;
}

void onDisconnect(uint16_t conn_handle, uint8_t reason) {
  (void)conn_handle;
  disconnectReason = reason;
  connected = false;
  connHandle = BLE_CONN_HANDLE_INVALID;
  ledOverride = LED_OVR_AUTO;
}

// Raw SoftDevice events: the radio reports how many notifications it has actually sent.
void onBleEvent(ble_evt_t* evt) {
  if (evt->header.evt_id == BLE_GATTS_EVT_HVN_TX_COMPLETE) {
    int32_t n = evt->evt.gatts_evt.params.hvn_tx_complete.count;
    int32_t left = __atomic_sub_fetch(&inFlight, n, __ATOMIC_SEQ_CST);
    if (left < 0) __atomic_store_n(&inFlight, 0, __ATOMIC_SEQ_CST);
  }
}

void onFeedbackWrite(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  (void)conn_hdl;
  (void)chr;
  if (len < 2) return;
  if (data[0] == CMD_LED_OVERRIDE && data[1] <= LED_OVR_OFF) {
    ledOverride = data[1];
  }
  // Unknown commands are ignored on purpose (forward compatibility).
}

void requestConnectionParameters(uint16_t handle) {
  ble_gap_conn_params_t p;
  p.min_conn_interval = CONN_INTERVAL_MIN_UNITS;
  p.max_conn_interval = CONN_INTERVAL_MAX_UNITS;
  p.slave_latency     = CONN_PERIPHERAL_LATENCY;
  p.conn_sup_timeout  = CONN_SUP_TIMEOUT_UNITS;
  uint32_t err = sd_ble_gap_conn_param_update(handle, &p);
  DBG("[ble] asked for 15-30 ms interval: %s\n", err == NRF_SUCCESS ? "sent" : "FAILED");
}

void startAdvertising() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(dialService);   // 128-bit UUID in the advertising packet
  Bluefruit.ScanResponse.addName();                // name goes in the scan response

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(ADV_FAST_INTERVAL_UNITS, ADV_SLOW_INTERVAL_UNITS);
  Bluefruit.Advertising.setFastTimeout(ADV_FAST_TIMEOUT_S);
  Bluefruit.Advertising.start(0);                  // 0 = advertise forever
}

void sendPendingBattery() {
  if (pendingBattery == 0xFF) return;
  if (!connected) { pendingBattery = 0xFF; return; }
  if (!reserveSlot()) return;                      // try again next loop
  if (!bas.notify(pendingBattery)) releaseSlot();  // false = not subscribed (value still stored)
  pendingBattery = 0xFF;
}

}  // namespace

void bleBegin() {
  // We drive the RGB LED ourselves.
  Bluefruit.autoConnLed(false);

  // Let up to 8 notifications wait in the radio's buffer instead of 1.
  Bluefruit.configPrphConn(BLE_GATT_ATT_MTU_DEFAULT, BLE_GAP_EVENT_LENGTH_DEFAULT,
                           kHvnSlots, BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT);

  if (!Bluefruit.begin(1, 0)) {   // 1 peripheral connection, 0 central
    DBG("[ble] ERROR: Bluefruit.begin() failed\n");
  }
  Bluefruit.setTxPower(BLE_TX_POWER_DBM);

  // Preferred connection parameters (also Apple-compliant; iOS ignores them, the
  // explicit request after connecting is what counts).
  Bluefruit.Periph.setConnInterval(CONN_INTERVAL_MIN_UNITS, CONN_INTERVAL_MAX_UNITS);
  Bluefruit.Periph.setConnSupervisionTimeout(CONN_SUP_TIMEOUT_UNITS);

  // "DIAL-XXXX": last 2 bytes of the device address, as shown in AA:BB:CC:DD:EE:FF order
  uint8_t mac[6];
  Bluefruit.getAddr(mac);
  snprintf(deviceName, sizeof(deviceName), "DIAL-%02X%02X", mac[1], mac[0]);
  Bluefruit.setName(deviceName);

  Bluefruit.Periph.setConnectCallback(onConnect);
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);
  Bluefruit.setEventCallback(onBleEvent);

  dis.setManufacturer(DIS_MANUFACTURER);
  dis.setModel(DIS_MODEL);
  dis.setFirmwareRev(FW_VERSION);
  dis.begin();

  bas.begin();
  bas.write(100);

  // The service must begin() before its characteristics.
  dialService.begin();

  inputChr.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ);
  inputChr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  inputChr.setFixedLen(4);
  inputChr.begin();
  uint8_t zero[4] = { 0, 0, 0, 0 };
  inputChr.write(zero, sizeof(zero));

  feedbackChr.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
  feedbackChr.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  feedbackChr.setFixedLen(2);
  feedbackChr.setWriteCallback(onFeedbackWrite);
  feedbackChr.begin();

  startAdvertising();
  DBG("[ble] advertising as %s\n", deviceName);
}

bool bleConnected()  { return connected; }
bool bleSubscribed() { return connected && inputChr.notifyEnabled(); }
uint8_t bleLedOverride() { return ledOverride; }

bool bleHasPending(uint8_t event) {
  for (uint8_t i = qHead; i != qTail; i = (i + 1) % kQueueSize) {
    if (queue[i].b[0] == event) return true;
  }
  return false;
}

bool bleQueueEvent(uint8_t event, int8_t value, uint8_t flags) {
  if (!bleSubscribed()) return false;      // nobody listening: drop, never buffer stale input
  if (queueFull()) {
    DBG("[ble] queue full, event %u dropped\n", event);
    return false;
  }
  Event& e = queue[qTail];
  e.b[0] = event;
  e.b[1] = (uint8_t)value;
  e.b[2] = flags;
  e.b[3] = 0;                              // sequence is stamped when actually sent
  qTail = (qTail + 1) % kQueueSize;
  return true;
}

void bleService(uint32_t now) {
  // New connection? Reset per-connection state. (Done here, not in the callback,
  // so the queue and sequence are only ever touched by loop().)
  uint32_t count = connectionCount;
  if (count != handledConnection) {
    handledConnection = count;
    queueClear();
    sequence = 0;
    paramsRequested = false;
    intervalLogged = false;
    DBG("[ble] connected\n");
  }

  if (!connected) {
    if (disconnectReason >= 0) {
      // 0x13 = phone closed it, 0x08 = link lost (out of range / timeout), 0x16 = we closed it
      DBG("[ble] disconnected, reason 0x%02X\n", (unsigned)disconnectReason);
      disconnectReason = -1;
    }
    queueClear();
    return;
  }

  // Signed difference: the connect callback can run after `now` was read, which would
  // otherwise wrap to a huge number and fire the request immediately.
  int32_t since = (int32_t)(now - connectedAtMs);
  if (!paramsRequested && since >= (int32_t)CONN_PARAM_REQUEST_DELAY_MS) {
    paramsRequested = true;
    requestConnectionParameters(connHandle);
  }
  if (!intervalLogged && since >= (int32_t)(CONN_PARAM_REQUEST_DELAY_MS + 3000)) {
    intervalLogged = true;
    BLEConnection* c = Bluefruit.Connection(connHandle);
    if (c) {
      uint16_t units = c->getConnectionInterval();
      DBG("[ble] connection interval now %u.%02u ms\n", (units * 125) / 100, (units * 125) % 100);
    }
  }

  if (!inputChr.notifyEnabled()) {
    queueClear();
  } else {
    while (!queueEmpty()) {
      if (!reserveSlot()) break;           // radio buffer full: keep the event, retry next loop
      Event& e = queue[qHead];
      e.b[3] = sequence;
      if (!inputChr.notify(e.b, sizeof(e.b))) {
        releaseSlot();
        break;                             // not sent; same event, same sequence, next loop
      }
      DBG("[evt] %02X %02X %02X %02X\n", e.b[0], e.b[1], e.b[2], e.b[3]);
      sequence++;
      qHead = (qHead + 1) % kQueueSize;
    }
  }

  sendPendingBattery();
}

void bleSetBatteryLevel(uint8_t percent) {
  bas.write(percent);                      // readable at any time
  if (connected) pendingBattery = percent; // notified from bleService() when a slot is free
}

void bleShutdown() {
  uint16_t h = connHandle;
  Bluefruit.Advertising.restartOnDisconnect(false);
  if (connected && h != BLE_CONN_HANDLE_INVALID) {
    Bluefruit.disconnect(h);               // the phone sees the drop now, not after the 4 s timeout
    uint32_t start = millis();
    while (connected && (millis() - start) < 300) delay(5);
  }
  Bluefruit.Advertising.stop();
}
