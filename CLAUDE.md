# dial-firmware

Firmware for a Bluetooth LE camera dial controller (2 rotary encoders, 5 buttons) that talks to
the iPhone app in `~/dial-app`.

## Toolchain
- Board: Seeed XIAO nRF52840.
- Core: "Seeed nRF52 Boards" (`Seeeduino:nrf52`, Adafruit-based, Bluefruit BLE library).
  FQBN `Seeeduino:nrf52:xiaonRF52840`. Never use the "mbed-enabled" core.
- Library: Bounce2.
- Compile: `arduino-cli compile --fqbn Seeeduino:nrf52:xiaonRF52840 ~/dial-firmware`
  - arduino-cli is not on PATH; it is bundled at
    `/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli`.
  - The core's UF2 step calls `python`; with pyenv global = system that fails (exit 127), so
    prefix the command with `PYENV_VERSION=3.12.11`.
  - Expected: ~125 KB (15%) program storage, no errors.

## Layout
- This repo root is the Arduino sketch folder (`dial-firmware.ino` at top level; never nest a
  `dial-firmware/dial-firmware` folder).
- All settings (pins, timings, UUIDs, version) live in `dial_config.h`; event and command codes
  are in `protocol.h`.
- `PROTOCOL.md` is the contract with `~/dial-app` and must stay byte-identical in both repos.
  Any protocol change updates both copies plus `dial_config.h` / `protocol.h`, and later the
  Swift code.

## Hard rules
- `VBAT_ENABLE` (P0.14) is driven LOW as the first thing in `setup()` and must never be set HIGH
  (Seeed warns it can damage P0.31 while charging).
- BLE connection parameters follow Apple's accessory guidelines (min interval >= 15 ms,
  max >= min + 15 ms). The firmware requests 15-30 ms, never 7.5 ms.
- `notify()` must never block `loop()`; `ble_link.cpp` counts in-flight notifications for this.
- The sequence byte is stamped only when a notify succeeds.

## Project tracking
The plan and status for the whole project (both repos) live in `~/dial-app/docs/`:
`PLAN.md` (checklist by phase), `STATUS.md` (snapshot), `DECISIONS.md`, `LOG.md`.
- At the start of each session, read `~/dial-app/docs/STATUS.md` and the current phase in `PLAN.md`.
- After completing a step, tick it in `PLAN.md`, update `STATUS.md`, and append a dated entry to
  `LOG.md`. Record any design choice in `DECISIONS.md`.
- Never tick a box unless the evidence exists (a file, a commit, a measured number). Steps that
  need hardware stay unticked until Diego reports the result.
- Those files are committed in the `dial-app` repo, not here.
- Commands: `/status`, `/checkpoint`, `/gaps`.

Don't tag `fw-0.1.0` until the Phase 3 checklist in README.md passes.
