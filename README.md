# dial-firmware

Firmware for the iPhone Camera Dial Controller: a Seeed XIAO nRF52840 that reads two rotary
encoders and five buttons and sends them to the iPhone over Bluetooth LE.
Version **0.1.0**. Protocol: see `PROTOCOL.md` (keep an identical copy in `dial-app`).

Built and compiled against **Seeed nRF52 Boards 1.1.13** and **Bounce2 2.72**, with no warnings.

## Files

| File | What it does |
|---|---|
| `dial-firmware.ino` | `setup()` and `loop()`; ties the modules together |
| `dial_config.h` | Every setting: pins, timings, UUIDs, version, debug switches |
| `protocol.h` | Event codes and command codes from `PROTOCOL.md` |
| `encoders.*` | Reads both dials with interrupts and counts detents |
| `buttons.*` | Debounces the 5 switches; detects mode short/long press |
| `ble_link.*` | Bluetooth services, advertising, connection parameters, event queue |
| `battery.*` | Battery voltage and percentage, Battery Service |
| `status_led.*` | Blue blink / solid blue / red low battery |
| `power.*` | 10-minute idle timer and system-off sleep |

---

## Part A: today, no hardware needed

### A1. Install the board package

1. Arduino IDE 2.x → **Arduino IDE > Settings** (macOS).
2. In **Additional boards manager URLs** paste:
   `https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json`
   (if something is already there, put a comma between them). Click OK.
3. **Tools > Board > Boards Manager**, search `seeed nrf52`.
4. Install **Seeed nRF52 Boards**. Do **not** install "Seeed nRF52 mbed-enabled Boards";
   that one has a different Bluetooth library and this code will not compile on it.
5. **Tools > Board > Seeed nRF52 Boards > Seeed XIAO nRF52840** (not the "Sense" or "Plus" ones).

### A2. Install Bounce2

**Tools > Manage Libraries**, search `Bounce2`, install **Bounce2 by Thomas O Fredericks**.

### A3. Put the code in your repo and open it

The repo folder itself is the sketch folder. Copy the **files inside** the unzipped folder
into the top level of your local `dial-firmware` repo, not the folder itself.

Right:
```
dial-firmware/            <- your repo (contains the hidden .git folder)
├── dial-firmware.ino
├── dial_config.h
├── ...
├── PROTOCOL.md
└── README.md
```
Wrong (a folder inside the repo):
```
dial-firmware/
└── dial-firmware/
    └── dial-firmware.ino
```

1. Double-click `dial-firmware.zip` in Downloads; macOS creates a `dial-firmware` folder there.
2. Open that folder, press **Cmd+A**, and drag all 17 files into your repo folder.
   If Finder asks about replacing `README.md`, choose **Replace**.
3. In Arduino IDE, **File > Open** → `dial-firmware.ino` inside your repo folder. Every
   `.h`/`.cpp` file opens as a tab. If Arduino offers to "create a folder and move the file",
   click **Cancel**: it means your repo folder is not named exactly `dial-firmware`.

### A4. Compile without a board

Click **Verify** (the check-mark button). No board needs to be plugged in. The output should
end with something like `Sketch uses 125988 bytes (15%)`. If you get an error, paste the
whole red text back to Claude.

### A5. Commit

In Terminal, from the repo folder:
```
git add .
git commit -m "Firmware 0.1.0: protocol, encoders, buttons, BLE, battery, power"
git push -u origin HEAD
```
Also copy `PROTOCOL.md` into the top level of the `dial-app` repo and commit it there.
Do not tag 0.1.0 yet; that happens when the hardware checklist passes.

---

## Part B: when the parts arrive (no soldering, no multimeter)

### B1. Finish the Phase 2 checks first

1. Plug the XIAO into the Mac with the USB-C cable. **File > Examples > 01.Basics > Blink**,
   change `LED_BUILTIN` to `LED_BLUE`, **Upload**. The LED should blink.
   If upload fails, double-tap the tiny reset button on the XIAO (it enters bootloader mode and
   shows up as a USB drive), then pick the port again under **Tools > Port** and upload.
2. Turn each encoder by hand through a full turn: 24 even clicks, no dead spots.

### B2. Wire the breadboard

Unplug USB while wiring. Put the XIAO across the centre gap of the breadboard, USB end facing out.
Run a wire from the XIAO **GND** pin to the breadboard's blue (−) rail. Everything below that
says "GND" goes to that rail.

Check every pin against the **pinout diagram on the Seeed wiki** before powering up.

| Part | Its pin | Goes to |
|---|---|---|
| Front encoder | A (one outer pin of the 3-pin side) | D0 |
| | C (middle pin of the 3-pin side) | GND |
| | B (other outer pin of the 3-pin side) | D1 |
| | Switch pins (2-pin side) | one to D2, other to GND |
| Rear encoder | A (outer pin) | D3 |
| | C (middle pin) | GND |
| | B (other outer pin) | D4 |
| | Switch pins | one to D5, other to GND |
| Shutter, stage 1 (half) | switch | D6 and GND |
| Shutter, stage 2 (full) | switch | D7 and GND |
| Mode button | switch | D8 and GND |

Notes:
- Encoders: the two wide metal mounting tabs on the sides won't go into breadboard holes.
  Bend them flat or up with pliers. Only the 5 pins go in the breadboard.
- Shutter on the breadboard: two ordinary tactile switches, one for D6 and one for D7, are
  enough. To "full press", press the D6 one and then the D7 one while still holding the first.
  If you bought the Tindie ALPS breakout and its header pins come loose, that needs soldering;
  use the two tactile switches until the iron arrives.
- Tactile switches have 4 legs in 2 connected pairs. Use two diagonal legs (opposite corners)
  so you always get the switched pair.
- Which outer encoder pin is A doesn't matter: if a dial counts backwards you flip it in software.
- No resistors are needed anywhere; the firmware turns on the chip's internal pull-ups.
- Do **not** connect the battery yet.

### B3. Upload and watch the Serial Monitor

1. Plug in USB, **Tools > Port** → the XIAO, **Upload**.
2. **Tools > Serial Monitor**, set **115200 baud**.
3. Press the XIAO reset button once to see the boot lines (or set `DEBUG_WAIT_FOR_SERIAL_MS`
   to `3000` in `dial_config.h` and upload again). You should see:
   ```
   === Dial Controller firmware 0.1.0 ===
   [pwr] reset button
   [enc] rest state front=11 rear=11
   [ble] advertising as DIAL-1A2B
   [bat] 4.2xx V  100%
   ```
   The battery line means nothing until a battery is attached.
4. The LED should blink blue once a second.
5. Turn the front dial clockwise one click: `[dial] front +1  total 1`. Press each button:
   `[in] event 5 value 0 flags 1 (not sent: no phone subscribed)` and so on.
   If clockwise prints `-1`, set `FRONT_DIAL_INVERT` (or `REAR_DIAL_INVERT`) to `true` in
   `dial_config.h` and upload again.

### B4. Test with nRF Connect on the iPhone (Step 3.4)

1. Open nRF Connect, **Scanner**, find `DIAL-xxxx`, **Connect**. The LED turns solid blue.
2. Open the service `188D0001-…`, find the characteristic `188D0002-…`, and turn on
   notifications (the subscribe / down-arrows button).
3. Turn the front dial one click clockwise: the value shows `0x01010000`. One click back:
   `0x01FF0001`. That is `event, value, flags, sequence`; see the examples in `PROTOCOL.md`.
4. Test the Feedback characteristic: on `188D0003-…` write the bytes `0102`. The LED turns
   green. Write `0100` to go back to automatic.
5. Record the 20-second screen capture the guide asks for.

---

## Part C: when the multimeter and soldering iron arrive

You also need a **JST-PH 2.0 two-pin socket pigtail** (a short cable with the socket your
battery plugs into). It is not in the guide's parts list; order one now if you have not.

1. **Polarity first.** Plug the battery into the pigtail and measure across the pigtail's two
   wires with the multimeter on DC volts. Positive reading with red probe on the red wire =
   the colours are right. Negative = the vendor reversed them; label the wires accordingly.
   Write the result on tape on the battery.
2. Solder: pigtail **+** → slide switch middle pin; slide switch outer pin → XIAO **BAT+** pad
   on the back; pigtail **−** → XIAO **BAT−** pad.
3. Switch on, USB unplugged. The controller should start advertising (blue blink).
4. Plug USB back in, open the Serial Monitor, and compare the `[bat]` voltage with the
   multimeter across the BAT pads. Within about 0.05 V is fine. If it is always off by the same
   ratio, set `VBAT_CALIBRATION` in `battery.cpp` (meter ÷ printed).

---

## Done-when checklist (Phase 3)

| Check | How |
|---|---|
| Every control produces the right event in nRF Connect | Each button, both directions on both dials. Full shutter press = `05…01`, `06…01`, `06…00`, `05…00` |
| 10 fast turns = 240 ± 2 | Watch the Serial Monitor `total`. Press reset first so it starts at 0, spin 10 turns fast, read `total`. Repeat for the rear dial |
| Sequence never skips (2 minutes) | Random input for 2 minutes while subscribed; the last byte must count up by 1 every time in nRF Connect's log. The app in Phase 4 will check this automatically |
| Sleeps after 10 min, wakes on a button, LED states right | Set `IDLE_TIMEOUT_MS` to `30000UL` to test in 30 s instead of 10 min, then set it back. Dials do not wake it; buttons do |
| 8 hours connected on battery | Set `IDLE_TIMEOUT_MS` to 24 hours for this test, or it will sleep after 10 minutes and end the test. Leave nRF Connect connected overnight. Set it back afterwards |
| Committed, version 0.1.0 | `git tag fw-0.1.0 && git push --tags` |

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| `bluefruit.h: No such file` | Wrong board package (mbed) or wrong board selected |
| `Bounce2.h: No such file` | Bounce2 not installed |
| Upload says no device / port missing | Double-tap reset, choose the new port, upload again. Needed every time after the controller has gone to sleep |
| A dial counts 2 per click or 0 on some clicks | A/B or C wired to the wrong pins; C must be the middle pin to GND |
| A dial counts backwards | Set `FRONT_DIAL_INVERT` / `REAR_DIAL_INVERT` |
| Button events missing | Switch legs on the same side (always connected); use diagonal legs |
| LED behaves inverted | Set `LED_ACTIVE_LOW` to `false` |
| nRF Connect connects but no values | Notifications not enabled on `188D0002-…` |
| `[ble] connection interval now 30.00 ms` | Normal. iOS chose 30 ms from the 15–30 ms range; it may choose 15 ms later |
