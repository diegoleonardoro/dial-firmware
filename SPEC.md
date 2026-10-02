# Dial Controller: MVP Spec

**Version:** 0.1 (Phase 1) · **Date:** 2026-10-02 · **Status:** feel items marked *(provisional)* are revisited in Phase 5

## Target

- **Primary test phone:** iPhone 18 Pro on iOS 27.
- **Front dial on the 18 Pro:** real aperture, f/1.48 to f/4, main camera only.
- **Other iPhones (iOS 27):** supported. The aperture is fixed, so the front dial has no aperture role there. ISO is set with the ISO button on every model, not remapped onto the front dial.
- **Fixed-aperture rule:** the app treats a camera as fixed-aperture when `activeFormat.minLensAperture == maxLensAperture`. This includes the ultra wide and telephoto on an 18 Pro. On a fixed-aperture camera, Aperture priority is removed from the mode cycle and the HUD hides the f-number.

## Controls

| Control | Position | Action |
|---|---|---|
| Front dial | Front face, under index finger | Aperture in M and A modes; exposure compensation in S mode; idle otherwise |
| Rear dial | Back, under thumb | Shutter speed in M and S modes; exposure compensation in A and Auto modes |
| Shutter button, half press | Top | M mode: lock focus only. Other modes: lock focus and exposure. Release returns to continuous AF/AE |
| Shutter button, full press | Top | Photo mode: capture. Video mode: start/stop recording |
| ISO button (hold) | Top, behind shutter | While held, rear dial steps ISO. HUD highlights ISO |
| WB button (hold) | Top, behind ISO | While held, rear dial steps white-balance presets. HUD highlights WB |
| Mode button, short press | Top, rear edge | Cycle M → A → S → Auto. On fixed-aperture cameras: M → S → Auto |
| Mode button, long press (≥ 800 ms) | same | Toggle photo / video |
| Front dial press | — | Switch to Auto mode and reset exposure compensation to 0. ISO, WB and lens are unchanged |
| Rear dial press | — | Cycle lenses 0.5x → 1x → 2x → 4x, skipping any the phone lacks |
| Status LED | Light pipe, top | Blue blink: advertising. Solid blue: connected. Red: battery < 15 % |

## Dial behavior by mode

| Mode | Front dial | Rear dial | ISO | Exposure call |
|---|---|---|---|---|
| M (Manual) | Aperture | Shutter | User-set (ISO button) | aperture, duration, ISO all fixed |
| A (Aperture priority) | Aperture | EV comp | Auto | aperture fixed, `.autoExposureDuration`, `.autoISO` |
| S (Shutter priority) | EV comp | Shutter | Auto | `.autoLensAperture`, duration fixed, `.autoISO` |
| Auto | Idle | EV comp | Auto | all auto |
| M, fixed aperture | Idle | Shutter | User-set | duration, ISO fixed |

**Modifiers:**
- Holding the ISO or WB button overrides the rear dial's normal job. The front dial is idle while either button is held.
- Pressing ISO or WB in a mode where that setting is auto shows "AUTO" in the HUD and does nothing else.
- Every combination is checked with `supportsExposureModeCustom(lensAperture:duration:iso:)` before it is applied.

## Value lists (stepped one detent at a time, clamped to the active format)

- **Shutter (third stops, s):**
  - 1/8000, 1/6400, 1/5000, 1/4000, 1/3200, 1/2500, 1/2000, 1/1600, 1/1250, 1/1000
  - 1/800, 1/640, 1/500, 1/400, 1/320, 1/250, 1/200, 1/160, 1/125, 1/100
  - 1/80, 1/60, 1/50, 1/40, 1/30, 1/25, 1/20, 1/15, 1/13, 1/10
  - 1/8, 1/6, 1/5, 1/4, 0.3, 0.4, 0.5, 0.6, 0.8, 1
  - Trimmed to `minExposureDuration` … `maxExposureDuration`.
- **Aperture (18 Pro, third stops):**
  - f/1.48, 1.6, 1.8, 2.0, 2.2, 2.5, 2.8, 3.2, 3.5, 4.0
  - This includes the four `recommendedLensApertureStops`.
- **ISO (third stops):**
  - 32, 40, 50, 64, 80, 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400
  - Clamped to `minISO` … `maxISO`.
- **Exposure compensation:**
  - −3.0 to +3.0 EV in 1/3 EV steps.
  - Clamped to `minExposureTargetBias` … `maxExposureTargetBias`.
- **White balance presets:**
  - Auto, Daylight 5500 K, Cloudy 6500 K, Shade 7500 K, Tungsten 3200 K, Fluorescent 4000 K
  - Applied via `deviceWhiteBalanceGains(for:)` and `setWhiteBalanceModeLocked`.

## Feel defaults *(provisional)*

- **Dial direction:** Nikon. Rear dial clockwise = faster shutter. Front dial clockwise = smaller aperture (higher f-number).
- **Steps:** one detent = one third stop on every list.
- **Acceleration:** off. A fast spin moves one step per detent.
- **Lists:** stop at each end; they do not wrap.

## Form factor

- **Size:** grip about 40 × 90 × 30 mm. It may grow about 10 mm taller to fit the ISO and WB buttons; this is checked in Phase 6.
- **Position:** right of a horizontally held phone, for a right-handed user.
- **Mount:** spring phone clamp on a 1/4-20 stud printed into the grip, so it works with any case.

## Pins (XIAO nRF52840)

| Pin | Signal |
|---|---|
| D0 / D1 | Front encoder A / B |
| D2 | Front encoder push |
| D3 / D4 | Rear encoder A / B |
| D5 | Rear encoder push |
| D6 | Shutter stage 1 (half) |
| D7 | Shutter stage 2 (full) |
| D8 | Mode button |
| D9 | ISO button |
| D10 | WB button |
| Onboard | RGB status LED |

- **Pin budget:** all D0–D10 pins are used. Any further button needs an I²C port expander.
- **Firmware role:** the firmware reports raw press and release for the ISO and WB buttons. The app resolves the hold-plus-dial combinations, so changing what a button does never needs a firmware update.

## Each control in one sentence

- **Front dial:** sets the aperture when the camera has a variable iris and the mode lets you choose it; otherwise it adjusts exposure compensation in S mode and does nothing else.
- **Rear dial:** sets shutter speed in M and S, and exposure compensation in A and Auto.
- **ISO + rear dial:** holding ISO and turning the rear dial steps ISO in third stops.
- **WB + rear dial:** holding WB and turning the rear dial steps through the white-balance presets.
- **Half press:** locks focus (and exposure outside Manual) until you let go.
- **Full press:** takes the photo, or starts and stops recording in video mode.
- **Mode, short press:** cycles M → A → S → Auto.
- **Mode, long press:** switches between photo and video.
- **Front dial press:** drops the camera into Auto with zero exposure compensation.
- **Rear dial press:** switches to the next lens.
- **LED:** shows advertising, connected, or low battery.
