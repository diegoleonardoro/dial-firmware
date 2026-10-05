# Dial Controller BLE Protocol

Protocol version: **0.1.0** (matches firmware 0.1.0)

This file must be **identical in both repos** (`dial-firmware` and `dial-app`).
If you change a UUID or a byte here, change it in both code bases the same day,
then diff the strings against `dial_config.h` (firmware) and the Swift constants (app).

## Roles

- Controller (XIAO nRF52840) = BLE **peripheral** / GATT server
- iPhone app = BLE **central** / GATT client
- No pairing or bonding in 0.1.0. All characteristics are open.

## UUIDs

All three custom UUIDs share one random base and differ only in the 2nd 16-bit group
(`0001`, `0002`, `0003`). They were generated once with a random v4 UUID; keep them.

| Item | UUID | Properties |
|---|---|---|
| Dial Controller Service | `188D0001-8E0D-4A9D-9B04-830DCBEB93B8` | primary service, advertised |
| Input Event characteristic | `188D0002-8E0D-4A9D-9B04-830DCBEB93B8` | Notify, Read |
| Feedback characteristic | `188D0003-8E0D-4A9D-9B04-830DCBEB93B8` | Write, Write Without Response |
| Battery Service | `0x180F` (standard) | |
| Battery Level characteristic | `0x2A19` (standard) | Read, Notify, 1 byte, 0 to 100 |
| Device Information Service | `0x180A` (standard) | |
| Manufacturer Name | `0x2A29` | Read, `"DIY"` |
| Model Number | `0x2A24` | Read, `"Dial Controller"` |
| Firmware Revision | `0x2A26` | Read, `"0.1.0"` |
| Serial Number | `0x2A25` | Read, the chip's unique ID (added automatically by the Bluefruit library) |
| Software Revision | `0x2A28` | Read, the board-package version, e.g. `"1.1.13"` (added automatically) |

For the app's `Info.plist` (`NSAccessorySetupBluetoothServices`) use the service UUID
**in upper case**, exactly as written above.

## Advertising

| Field | Value |
|---|---|
| Advertising packet | Flags (LE General Discoverable, BR/EDR not supported) + the 128-bit Dial Controller Service UUID |
| Scan response | Complete local name `DIAL-XXXX`, where `XXXX` is the last 2 bytes of the device address in hex |
| Interval | 20 ms for the first 30 s, then 152.5 ms (both from Apple's recommended list) |
| Restart | Advertising restarts automatically after a disconnect |

The name is in the scan response because the 128-bit UUID plus the name do not fit in one
31-byte advertising packet. iOS scans actively, so it always sees both.

## Connection parameters

About 1 s after connecting, the controller sends an L2CAP Connection Parameter Update Request:

| Parameter | Value | Apple rule it satisfies |
|---|---|---|
| Interval min | 15 ms | Interval Min ≥ 15 ms, multiple of 15 ms |
| Interval max | 30 ms | Interval Max ≥ Interval Min + 15 ms |
| Peripheral latency | 0 | ≤ 30 |
| Supervision timeout | 4 s | between 2 and 6 s, and > Interval Max × (latency + 1) × 3 |

Note: the build guide's AI prompt says "7.5 to 15 ms". iOS rejects intervals below 15 ms for a
non-HID accessory, so the firmware asks for 15 to 30 ms instead. iOS decides the final value.

## Input Event characteristic (device → phone)

Every notification is exactly **4 bytes**:

| Byte | Name | Type | Meaning |
|---|---|---|---|
| 0 | `event` | uint8 | Event code, see table below |
| 1 | `value` | **int8** (signed) | Dial events: number of detents turned. Buttons: always 0 |
| 2 | `flags` | uint8 | bit 0: 1 = pressed, 0 = released (button events 3 to 6). Bits 1 to 7 reserved, always 0 |
| 3 | `sequence` | uint8 | Increments by 1 on every notification, wraps 255 → 0 |

### Event codes

| Code | Event | `value` | `flags` bit 0 | Sent when |
|---|---|---|---|---|
| `0x01` | Front dial delta | signed detents, usually ±1 | 0 | at most once every 15 ms while turning |
| `0x02` | Rear dial delta | signed detents, usually ±1 | 0 | at most once every 15 ms while turning |
| `0x03` | Front dial press | 0 | 1 press / 0 release | on press and on release |
| `0x04` | Rear dial press | 0 | 1 press / 0 release | on press and on release |
| `0x05` | Shutter half press (stage 1) | 0 | 1 press / 0 release | on press and on release |
| `0x06` | Shutter full press (stage 2) | 0 | 1 press / 0 release | on press and on release |
| `0x07` | Mode button short press | 0 | 0 | on release, if held < 400 ms |
| `0x08` | Mode button long press | 0 | 0 | as soon as it has been held 800 ms (not on release) |

A mode press held between 400 and 800 ms sends nothing, on purpose, so an uncertain press
does nothing instead of the wrong thing.

Each shutter stage is debounced on its own, so in rare cases `06 pressed` can arrive a few
milliseconds before `05 pressed`. The app should treat a full press as implying a half press.

### Dial direction and size

- Positive `value` = clockwise, looking at the knob face. Each dial has an invert switch in
  `dial_config.h` (`FRONT_DIAL_INVERT`, `REAR_DIAL_INVERT`) in case the wiring makes it backwards.
- Turns are summed for 15 ms and sent as one notification, so a fast spin sends larger values
  (for example `+3`) instead of more packets. The app should apply `value` steps, not 1 step per packet.
- The firmware never sends `value = 0` for a dial event.
- At most one unsent event per dial is ever queued. If the radio is busy, further detents stay
  in the encoder counter and go out together in the next event, so no detent is dropped.

### Examples

| Bytes | Meaning |
|---|---|
| `01 01 00 07` | Front dial, +1 detent (clockwise), sequence 7 |
| `01 FF 00 08` | Front dial, −1 detent (0xFF = −1), sequence 8 |
| `02 03 00 09` | Rear dial, +3 detents in one 15 ms window, sequence 9 |
| `05 00 01 0A` | Shutter half press, pressed |
| `06 00 01 0B` | Shutter full press, pressed |
| `06 00 00 0C` | Shutter full press, released |
| `05 00 00 0D` | Shutter half press, released |

### Sequence rules for the app

- The sequence restarts at `00` on every new connection.
- Events produced while nothing is subscribed are dropped, not buffered.
- If `(sequence − lastSequence) mod 256 ≠ 1`, a notification was lost; log it.

## Feedback characteristic (phone → device)

Every write is exactly **2 bytes**: `command`, `argument`. Unknown commands are ignored.

| Command | Name | Argument |
|---|---|---|
| `0x01` | LED override | `0x00` automatic status LED (default), `0x01` red, `0x02` green, `0x03` blue, `0x04` white, `0x05` off |

Example: writing `01 02` turns the LED solid green until `01 00` is written or the
connection drops. Reserved for later: `0x02` haptic pulse.

## Battery Service

Battery Level (0 to 100) is updated every 60 s and once at connection. The controller
sends a notification when the app has subscribed. The percentage comes from a LiPo
voltage table, so treat it as a rough gauge.

## Power behaviour the app should expect

- After 10 minutes with no input the controller enters system-off sleep and the connection
  drops (supervision timeout). Any button press wakes it; it reboots and advertises again.
- Turning a dial does **not** wake it from sleep; only the buttons do.
