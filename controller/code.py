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


def draw_notes(held):
    matrix.fill(0)
    for x in range(8):
        matrix[x, 7] = 1
        if held[x]:
            for y in range(1, 7):
                matrix[x, y] = 1
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

all_notes_off()
draw_notes(stable)
update_pixel(stable)

while True:
    now = time.monotonic()
    display_changed = False

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
            display_changed = True

    if display_changed:
        draw_notes(stable)
        update_pixel(stable)

    time.sleep(0.002)
