#!/usr/bin/env python3
# course: SWE4211 Real-Time Systems
# laboratory: 2
# date: revised August 2026 (original July 2018)
# username: wschilling
# name: Walter Schilling
# description: Demonstrates input to output latency by driving an LED from a
#              pushbutton.  The sampling loop is scheduled against an absolute
#              time base, and the program reports both the achieved sampling
#              period and the sample to actuation latency so that the
#              contribution of the polling period can be measured directly.

import sys
import time

import lgpio

# LED polarity.  Active low means driving the pin low sinks current through
# the LED and illuminates it.  VERIFY THIS AGAINST THE LAB WIRING: the 2018
# blink program and the 2018 control program disagreed on this point.
LED_ON = 0
LED_OFF = 1

# Button polarity.  The internal pull up holds the pin high when the button is
# released, so a press pulls the pin low.
BUTTON_PRESSED = 0

USAGE = ("usage: {0} <led_bcm_pin> <button_bcm_pin> <sample_period_in_ms>\n"
         "       a sample period of 0 samples continuously with no delay")


def parse_arguments(argv):
    """Validate the command line and return (led, button, period_seconds)."""
    if len(argv) != 4:
        raise SystemExit(USAGE.format(argv[0]))

    try:
        led = int(argv[1])
        button = int(argv[2])
        period_ms = float(argv[3])
    except ValueError:
        raise SystemExit(USAGE.format(argv[0]))

    for pin, label in ((led, "LED"), (button, "Button")):
        if not 0 <= pin <= 27:
            raise SystemExit(
                "{0} pin must be a BCM GPIO number in 0..27.".format(label))
    if led == button:
        raise SystemExit("The LED and button must be on different pins.")
    if period_ms < 0.0:
        raise SystemExit("Sample period cannot be negative.")

    return led, button, period_ms / 1000.0


def open_header_chip():
    """Open the gpiochip that backs the 40 pin header.

    On the Pi 5 the header is exposed by the RP1 southbridge, which has
    appeared as both gpiochip0 and gpiochip4 depending on kernel version.
    On earlier boards it is the BCM controller at gpiochip0.
    """
    for chip in (0, 4):
        try:
            return lgpio.gpiochip_open(chip)
        except lgpio.error:
            continue
    raise SystemExit(
        "Could not open a GPIO chip; is the user in the gpio group?")


def main(argv):
    led_pin, button_pin, period = parse_arguments(argv)

    free_running = (period == 0.0)

    if free_running:
        print("LED on BCM {0}, button on BCM {1}, sampling continuously.".format(
            led_pin, button_pin))
        print("Note: this busy polls and will hold one core at 100 percent.")
    else:
        print("LED on BCM {0}, button on BCM {1}, sampling every {2:.3f} ms.".format(
            led_pin, button_pin, period * 1000.0))
    print("Press control C to stop.")

    handle = open_header_chip()
    lgpio.gpio_claim_output(handle, led_pin, LED_OFF)
    # The internal pull up removes the need for an external resistor and gives
    # the input a defined level when the button is open.
    lgpio.gpio_claim_input(handle, button_pin, lgpio.SET_PULL_UP)

    sample_count = 0
    worst_period_error = 0.0
    worst_response = 0.0
    total_response = 0.0
    edge_count = 0
    last_state = None
    last_sample_at = None
    shortest_interval = float("inf")
    longest_interval = 0.0
    origin = time.monotonic()

    try:
        while True:
            if not free_running:
                # Each deadline is an offset from a single origin rather than
                # an increment on the previous deadline, so sampling error
                # does not accumulate over a long run.
                deadline = origin + sample_count * period

                remaining = deadline - time.monotonic()
                if remaining > 0.0:
                    time.sleep(remaining)

            # Timestamp the sample itself, not the top of the loop, so the
            # reported latency covers only read, decide, and write.
            sampled_at = time.monotonic()
            button_state = lgpio.gpio_read(handle, button_pin)

            if button_state != last_state:
                lgpio.gpio_write(
                    handle, led_pin,
                    LED_ON if button_state == BUTTON_PRESSED else LED_OFF)

                response = time.monotonic() - sampled_at
                total_response += response
                if response > worst_response:
                    worst_response = response
                edge_count += 1
                last_state = button_state

            # The interval between consecutive samples is meaningful in both
            # modes: it is the achieved period when scheduled, and the raw
            # loop time when free running.
            if last_sample_at is not None:
                interval = sampled_at - last_sample_at
                if interval > longest_interval:
                    longest_interval = interval
                if interval < shortest_interval:
                    shortest_interval = interval
            last_sample_at = sampled_at

            if not free_running:
                period_error = sampled_at - deadline
                if period_error > worst_period_error:
                    worst_period_error = period_error

            sample_count += 1

    except KeyboardInterrupt:
        print("\nW: interrupt received, stopping...")

    finally:
        elapsed = time.monotonic() - origin
        print("Samples taken:               {0}".format(sample_count))
        if sample_count:
            print("Achieved sample period:      {0:.3f} ms".format(
                (elapsed / sample_count) * 1000.0))
        if sample_count > 1:
            print("Shortest sample interval:    {0:.3f} ms".format(
                shortest_interval * 1000.0))
            print("Longest sample interval:     {0:.3f} ms".format(
                longest_interval * 1000.0))
        if not free_running:
            print("Worst case sampling delay:   {0:.3f} ms".format(
                worst_period_error * 1000.0))
        if edge_count:
            print("State changes detected:      {0}".format(edge_count))
            print("Worst sample to LED write:   {0:.3f} ms".format(
                worst_response * 1000.0))
            print("Mean sample to LED write:    {0:.3f} ms".format(
                (total_response / edge_count) * 1000.0))
        print("Worst case button to LED latency is bounded by the longest")
        print("sample interval plus the sample to write time reported above.")

        # Return the hardware to a known, safe state.
        lgpio.gpio_write(handle, led_pin, LED_OFF)
        lgpio.gpio_free(handle, led_pin)
        lgpio.gpio_free(handle, button_pin)
        lgpio.gpiochip_close(handle)


if __name__ == "__main__":
    main(sys.argv)
