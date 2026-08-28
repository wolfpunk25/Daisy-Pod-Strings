"""Eight-note MIDI controller for Daisy Pod Strings.

The button GPIO tuple is the only wiring-dependent setting. Positions are
musical order 1..8, read row-major in the controller photo: the two-button row,
then the middle three-button row, then the lower three-button row.
"""

import time

import board
import busio
import digitalio
import neopixel
import usb_midi

from adafruit_ht16k33.matrix import Matrix8x8


# Physical musical positions 1..8. The current order preserves the numbering
# from the previous controller firmware and can be corrected after a button
# diagnostic without changing any other code.
BUTTON_PINS = (
    board.GP26,
    board.GP21,
    board.GP22,
    board.GP20,
    board.GP18,
    board.GP17,
    board.GP19,
    board.GP16,
)

FIRST_NOTE = 60
MIDI_CHANNEL = 0
DEBOUNCE_SECONDS = 0.018

NOTE_COLORS = (
    (255, 35, 15),
    (255, 100, 10),
    (255, 210, 15),
    (40, 230, 50),
    (10, 190, 220),
    (25, 90, 255),
    (120, 35, 255),
    (240, 30, 190),
)

# Large 7-row glyphs. A-H label the eight performance positions; they are not
# literal pitch names because the active scale is selected independently on the
# Pod. Rows use the least-significant bit as the leftmost pixel.
NOTE_GLYPHS = (
    (0x18, 0x24, 0x42, 0x42, 0x7E, 0x42, 0x42),  # A
    (0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C),  # B
    (0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C),  # C
    (0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78),  # D
    (0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E),  # E
    (0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40),  # F
    (0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C),  # G
    (0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42),  # H
)

START_GLYPH = (0x3C, 0x42, 0x40, 0x3C, 0x02, 0x42, 0x3C)  # S

i2c = busio.I2C(scl=board.GP1, sda=board.GP0, frequency=400000)
matrix = Matrix8x8(i2c, address=0x70)
matrix.auto_write = False
matrix.brightness = 0.32

pixel = neopixel.NeoPixel(board.GP23, 1, brightness=0.40, auto_write=False)

buttons = []
for pin in BUTTON_PINS:
    button = digitalio.DigitalInOut(pin)
    button.direction = digitalio.Direction.INPUT
    button.pull = digitalio.Pull.UP
    buttons.append(button)

usb_out = usb_midi.ports[1]
uart_out = busio.UART(tx=board.GP4, baudrate=31250)


def send_raw(data):
    try:
        usb_out.write(data)
    except Exception:
        pass
    uart_out.write(data)


def send_note(index, pressed):
    status = (0x90 if pressed else 0x80) | MIDI_CHANNEL
    velocity = 110 if pressed else 0
    send_raw(bytes((status, FIRST_NOTE + index, velocity)))


def all_notes_off():
    send_raw(bytes((0xB0 | MIDI_CHANNEL, 123, 0)))


def draw_glyph(rows):
    """Draw an upright glyph on the matrix as mounted in the enclosure."""
    matrix.fill(0)
    for y, row in enumerate(rows):
        for x in range(8):
            if row & (1 << x):
                # The matrix PCB is mounted 90 degrees clockwise relative to
                # the HT16K33 driver's coordinate system.
                matrix[y, 7 - x] = 1
    matrix.show()


def update_pixel(held):
    active = -1
    for index, pressed in enumerate(held):
        if pressed:
            active = index
    pixel[0] = NOTE_COLORS[active] if active >= 0 else (8, 2, 18)
    pixel.show()


stable = [not button.value for button in buttons]
candidate = list(stable)
changed_at = [time.monotonic()] * len(buttons)
held_order = [index for index, pressed in enumerate(stable) if pressed]
last_played = held_order[-1] if held_order else None

all_notes_off()
draw_glyph(NOTE_GLYPHS[last_played] if last_played is not None else START_GLYPH)
update_pixel(stable)

while True:
    now = time.monotonic()
    for index, button in enumerate(buttons):
        raw_pressed = not button.value
        if raw_pressed != candidate[index]:
            candidate[index] = raw_pressed
            changed_at[index] = now
        elif candidate[index] != stable[index] and (
            now - changed_at[index] >= DEBOUNCE_SECONDS
        ):
            stable[index] = candidate[index]
            send_note(index, stable[index])
            if stable[index]:
                if index in held_order:
                    held_order.remove(index)
                held_order.append(index)
                last_played = index
            elif index in held_order:
                held_order.remove(index)

            displayed = held_order[-1] if held_order else last_played
            draw_glyph(NOTE_GLYPHS[displayed])
            update_pixel(stable)

    time.sleep(0.002)
