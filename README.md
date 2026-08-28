# Daisy Pod Strings

TouchString adapted for a Daisy Pod and a homemade eight-button CircuitPython
MIDI controller. The sound engine comes from Synthux Academy's
[TouchString](https://github.com/Synthux-Academy/TouchString); its Simple Touch
hardware layer is replaced by the Pod controls and TRS MIDI input.

The design deliberately avoids shift buttons and button combinations.

## Quick install

- Flash the ready-built `firmware/DaisyPodStrings.bin` to the Pod at address
  `0x08000000` while it is in STM32 DFU mode.
- Copy `controller/code.py` to `CIRCUITPY/code.py`.

Build from source only if you want to change the instrument.

## Controls

### MIDI controller

- Buttons 1–8 play all eight notes in the selected scale.
- The 8×8 display shows button number/order and held notes as vertical bars.
- The NeoPixel follows the most recently held note.
- Note messages are sent simultaneously over USB MIDI and GP4 UART/TRS MIDI.
- MIDI notes are C4–G4 (`60`–`67`) on channel 1.

The photographed physical layout is interpreted row-by-row: two buttons on the
upper row, three on the middle row, and three on the lower row. The actual GPIO
order is isolated in `controller/code.py` as `BUTTON_PINS`; edit only that tuple
if a hardware test reveals a different wiring order.

### Daisy Pod

- **Button 1:** Direct → Arpeggiator → Latched arpeggiator
- **Button 2:** Amara → Oxalis → Pigmy scale
- **Encoder turn:** tempo down/up
- **Encoder press:** next two-knob parameter page
- **LED 1:** green = Direct, blue = Arpeggiator, magenta = Latch
- **LED 2:** parameter-page colour; briefly flashes scale colour after Button 2

Parameter pages use soft takeover: a parameter does not jump when the page
changes. Move a knob through its saved position to pick it up.

| LED 2 | Knob 1 | Knob 2 |
| --- | --- | --- |
| Orange | Brightness | Structure |
| Red | Damping | Drive |
| Green | Pattern density | Pattern shift |
| Cyan | Note variation | String variation |
| Violet | Reverb mix | Transpose |

The line input and SD card are intentionally unused in this first version.

## Controller installation

1. Back up the existing `CIRCUITPY/code.py`.
2. Copy `controller/code.py` to `CIRCUITPY/code.py`.
3. The controller reloads automatically.

The required CircuitPython libraries are the same ones already installed for
the controller: `neopixel` and `adafruit_ht16k33`.

## Daisy build

```sh
git submodule update --init
make libs -j8
make -j8
```

The resulting firmware is `build/DaisyPodStrings.bin`. With the Pod in DFU
mode, flash it with:

```sh
make program-dfu
```

## MIDI compatibility

The Pod listens on MIDI channel 1 through its hardware MIDI input. In addition
to notes 60–67 and MIDI clock, the original TouchString CC mapping remains
available:

| CC | Parameter |
| --- | --- |
| 70 | Brightness |
| 71 | Transpose |
| 72 | Structure |
| 73 | Pattern density |
| 74 | Note variation |
| 75 | String variation |
| 76 | Damping |
| 77 | Reverb mix |
| 78 | Pattern shift |
| 79 | Drive |
| 123 | Reset/all notes off |

## Credits

- [Synthux Academy](https://github.com/Synthux-Academy/TouchString) — original
  TouchString project and sound engine
- [Rosa Schuurmans](https://github.com/vitrinekast) — original libDaisy port and
  improvements
- [Vlad Litvinenko](https://github.com/bleeptools) — initial implementation
- Electro-Smith — Daisy Seed, Daisy Pod, libDaisy, and DaisySP
