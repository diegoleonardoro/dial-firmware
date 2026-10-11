# Breadboard layout (fw README B2, PLAN step 3.1)

Hole-by-hole wiring for the **400-point breadboard** (30 rows, columns a–j). Picture:
`docs/breadboard_map.png` (source `docs/breadboard_map.svg`). Progress through these steps is
tracked in `~/dial-app/docs/PLAN.md` (step 3.1) and `STATUS.md`.

USB unplugged the whole time. Battery not connected.

## Words

| Word | Meaning |
|---|---|
| Hole name | Letter (a–j, across the top) + row number (1–30, down the side), e.g. **a10** |
| Groove | The channel down the middle. Separates the a–e half from the f–j half |
| Row | The 5 holes a–e of one number are joined inside; so are f–j. Not joined across the groove |
| Blue (−) line | Long line of holes along each edge, all joined top to bottom. Used as ground. Red (+) lines are not used |
| GND (ground) | The XIAO's zero connection. Every dial and button connects a D pin to ground |
| Jumper wire | **Male–male** wire (metal pin on both ends) |

On this breadboard the **left blue line is the inner one** (next to column a) and the
**right blue line is the outer one**.

## Step 1: Position the breadboard

Row 1 at the top, column a on the left (printed letters read the right way up).

## Step 2: XIAO

Sticker up, USB port toward row 1, centred over the groove. Pins in rows 1–7, left row of pins
in the left half, right row in the right half. Pressed down until the black plastic strips touch.

| Left side (column a) | Right side (column j) |
|---|---|
| a1 = D0 | j1 = 5V (leave empty) |
| a2 = D1 | j2 = GND |
| a3 = D2 | j3 = 3V3 (leave empty) |
| a4 = D3 | j4 = D10 (empty for now) |
| a5 = D4 | j5 = D9 (empty for now) |
| a6 = D5 | j6 = D8 |
| a7 = D6 | j7 = D7 |

## Step 3: Ground (wires 1–2)

| Wire | From | To |
|---|---|---|
| 1 | j2 (GND) | right blue line |
| 2 | right blue line, just below row 30 | left blue line, just below row 30 |

## Step 4: Encoders (wires 3–12)

Bend the two wide metal side tabs flat. Knobs off. Each encoder sits across the groove:
3-leg side in the left half (one leg per row), 2-leg side in the right half. Middle leg = C,
outer legs = A and B (either way round), 2-leg side = S (push switch). If a leg lands one row off,
put its wire in the row it actually landed in.

Front encoder: A C B in rows 10 11 12, S in rows 10 and 12.

| Wire | From | To |
|---|---|---|
| 3 | a10 (A) | a1 (D0) |
| 4 | a11 (C) | left blue line |
| 5 | a12 (B) | a2 (D1) |
| 6 | j10 (S) | a3 (D2) |
| 7 | j12 (S) | right blue line |

Rear encoder: A C B in rows 15 16 17, S in rows 15 and 17.

| Wire | From | To |
|---|---|---|
| 8 | a15 (A) | a4 (D3) |
| 9 | a16 (C) | left blue line |
| 10 | a17 (B) | a5 (D4) |
| 11 | j15 (S) | a6 (D5) |
| 12 | j17 (S) | right blue line |

## Step 5: Buttons (wires 13–18)

Each 6 mm tactile switch sits across the groove, 2 legs per side. Always wire **diagonal
corners**. Label each with tape.

| Button | Legs in rows | Wire | From | To |
|---|---|---|---|---|
| HALF (shutter half, D6) | 20, 22 | 13 | a20 | a7 (D6) |
| | | 14 | j22 | right blue line |
| FULL (shutter full, D7) | 24, 26 | 15 | j24 | j7 (D7) |
| | | 16 | a26 | left blue line |
| MODE (D8) | 28, 30 | 17 | j28 | j6 (D8) |
| | | 18 | a30 | left blue line |

## Step 6: Check before USB

- 18 wires. Ground wires (1, 2, 4, 7, 9, 12, 14, 16, 18) in black if possible.
- Nothing in j1 (5V), j3 (3V3), j4, j5, or either red line. A wire in j1 or j3 is the only
  harmful mistake here.
- Take a photo from directly above and have Claude check it before plugging in USB.

Next: fw README **B3** (upload the firmware, Serial Monitor).
