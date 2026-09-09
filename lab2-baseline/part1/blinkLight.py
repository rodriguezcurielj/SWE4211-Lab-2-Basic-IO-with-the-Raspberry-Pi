#!/usr/bin/env python3
# course: SWE4211 Real-Time Systems
# laboratory: 2
# date: revised August 2026 (original July 2018)
# username: wschilling
# name: Walter Schilling
# description: Blinks an LED on the Raspberry Pi using the lgpio library, the
#              currently supported GPIO interface on Raspberry Pi OS.  Edges
#              are scheduled against an absolute time base so that period
#              error does not accumulate.  Functionally identical to the
#              wiringPi version; only the device access layer differs.

import sys
import time

import lgpio

# Active low wiring: driving the pin low sinks current through the LED.
LED_ON = 0
LED_OFF = 1

USAGE = "usage: {0} <bcm_pin> <blinks_per_second> <duration_in_seconds>"


def parse_arguments(argv):
    """Validate the command line and return (pin, rate, duration)."""
    if len(argv) != 4:
        raise SystemExit(USAGE.format(argv[0]))

    try:
        pin = int(argv[1])
        rate = float(argv[2])
        duration = float(argv[3])
    except ValueError:
        raise SystemExit(USAGE.format(argv[0]))

    if not 0 <= pin <= 27:
        raise SystemExit("Pin must be a BCM GPIO number in the range 0..27.")
    if rate <= 0.0:
        raise SystemExit("Blink rate must be greater than zero.")
    if duration <= 0.0:
        raise SystemExit("Duration must be greater than zero.")

    return pin, rate, duration


def open_header_chip():
    """Open the gpiochip that backs the 40 pin header.

    On the Pi 5 the header is exposed by the RP1 southbridge, which has
    appeared as both gpiochip0 and gpiochip4 depending on kernel version.
    On earlier boards it is the BCM controller at gpiochip0.  Probing in
    order keeps one script working across the whole lab inventory.
    """
    for chip in (0, 4):
        try:
            return lgpio.gpiochip_open(chip)
        except lgpio.error:
            continue
    raise SystemExit("Could not open a GPIO chip; is the user in the gpio group?")


def main(argv):
    pin, rate, duration = parse_arguments(argv)

    # One blink is an on interval followed by an off interval, so the interval
    # between consecutive edges is one half of the period.
    half_period = 0.5 / rate
    edge_count = int(round(duration * rate)) * 2

    print("Blinking BCM pin {0} at {1} Hz for {2} seconds "
          "({3} edges, {4:.3f} ms between edges).".format(
              pin, rate, duration, edge_count, half_period * 1000.0))

    handle = open_header_chip()
    lgpio.gpio_claim_output(handle, pin, LED_OFF)

    worst_lateness = 0.0
    total_lateness = 0.0

    try:
        # Every deadline is computed from a single origin.  Computing each
        # deadline as an offset from the start, rather than adding to the
        # previous one, keeps both scheduling error and floating point error
        # from accumulating across the run.
        origin = time.monotonic()

        for edge in range(edge_count):
            deadline = origin + edge * half_period

            remaining = half_period
            if remaining > 0.0:
                time.sleep(remaining)

            lgpio.gpio_write(handle, pin, LED_ON if edge % 2 == 0 else LED_OFF)

            lateness = time.monotonic() - deadline
            total_lateness += lateness
            if lateness > worst_lateness:
                worst_lateness = lateness

        print("Worst case edge lateness: {0:.3f} ms".format(
            worst_lateness * 1000.0))
        print("Mean edge lateness:       {0:.3f} ms".format(
            (total_lateness / edge_count) * 1000.0))

    except KeyboardInterrupt:
        print("\nW: interrupt received, stopping...")

    finally:
        # Leave the hardware in a known, safe state regardless of how the
        # loop terminated.
        lgpio.gpio_write(handle, pin, LED_OFF)
        lgpio.gpio_free(handle, pin)
        lgpio.gpiochip_close(handle)


if __name__ == "__main__":
    main(sys.argv)

