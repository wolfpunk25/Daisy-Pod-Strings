# TouchString
Plunk and bang on metal, plastic and wood. Add distortion and modulation

## Project Setup
```shell
$ git clone --recurse-submodules https://github.com/Synthux-Academy/TouchString.git
$ make libs -j8
$ make clean; make -j8
```

## Controls

### Touch Pads

There are **12 touch-sensitive pads** (numbered 0-11):

| Pad(s) | Function | Notes |
|--------|----------|-------|
| **0** | **Tempo Down** / **Previous Scale** | When TO pad (10) is held: Slow down arpeggiator<br>When CH pad (11) is held: Select previous scale |
| **1** | *empty* | Not currently assigned |
| **2** | **Tempo Up** / **Next Scale** | When TO pad (10) is held: Speed up arpeggiator<br>When CH pad (11) is held: Select next scale |
| **3-9** | **Note Pads** (7 notes) | Play notes from the selected scale<br>In arp mode: Add/remove notes from arpeggio<br>In latch mode: Toggle notes on/off |
| **10** | **TO Modifier** | Hold to enable tempo control and alternative knob functions |
| **11** | **CH Modifier** | Hold to enable scale selection |

### Knobs

**8 analog control knobs** mapped to pins S30-S37:

| Knob | Pin | Parameter | Description |
|------|-----|-----------|-------------|
| **1** | S30 | **Brightness** | Controls filter brightness/cutoff frequency |
| **2** | S31 | **Pitch** | Global pitch transpose/tuning |
| **3** | S32 | **Timbre** | Sound structure/harmonic content |
| **4** | S33 | **Density** / **Pattern Shift** | Default: Pattern density/complexity<br>When TO (pad 10) held: Shift pattern timing |
| **5** | S34 | **Notes** | Humanization of note timing/variation |
| **6** | S35 | **String Chance** / **Reverb Mix** | Default: Probability of string triggering<br>When TO (pad 10) held: Reverb wet/dry mix |
| **7** | S36 | **Drive** | Distortion/overdrive amount (reduces volume as drive increases) |
| **8** | S37 | **Damp** | String damping/decay time |

> **Note:** Some knobs have dual functions based on modifier pad state (TO/CH)

### Switches

**2 three-position switches** for mode selection:

| Switch | Positions | Function |
|--------|-----------|----------|
| **A** | **Down**: Off<br>**Center**: Arp On<br>**Up**: Latch | **Arpeggiator Mode**<br>Down: Direct note playing (no arp)<br>Center: Arpeggiator active, notes release when pads released<br>Up: Latching arpeggiator (notes toggle on/off) |
| **B** | *(Reserved)* | Not currently assigned |

### LED Indicator

The onboard LED indicates **latch mode status** (lit when latch is active).

## Configuration

Edit `config.h` to customize the instrument:

### Scales
Three built-in scales with 8 notes each:
- **Amara**: D3, A3, C4, D4, E4, F4, G4, A4
- **Oxalis**: F3, A3, Bb3, C4, D4, F4, A4, Bb4
- **Pigmy**: F3, G3, Ab3, C4, Eb4, F4, G4, Ab4


## Project Structure

```
TouchString/
├── app.cpp              # Main application entry point
├── config.h             # Configuration parameters
├── Makefile             # Build configuration
├── string/              # String synthesis and arpeggiator logic
├── touch/               # Touch interface (pads, knobs, switches)
├── ui/                  # User interface logic
└── lib/                 # External libraries
    ├── libDaisy/        # Daisy hardware abstraction
    └── DaisySP/         # DSP library
```