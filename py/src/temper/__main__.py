"""Main functions for temper program"""

from __future__ import annotations

import time

from RPi import GPIO

LED_PIN = 26     # BCM pin 26
WAIT_TIME = 0.5  # Wait 0.5 seconds between on/off


def setup() -> None:
    """Set up the Rpi for blinking"""
    # Numbers GPIOs by physical location
    GPIO.setmode(GPIO.BCM)
    # Set LedPin's mode is output
    GPIO.setup(LED_PIN, GPIO.OUT)
    # Set LedPin high(+3.3V) to off led
    GPIO.output(LED_PIN, GPIO.HIGH)


def loop() -> None:
    """Blink the led"""
    print("Led is on.")
    # led on
    GPIO.output(LED_PIN, GPIO.LOW)
    time.sleep(WAIT_TIME)
    print("Led is off.")
    # led off
    GPIO.output(LED_PIN, GPIO.HIGH)
    time.sleep(WAIT_TIME)


def destroy() -> None:
    """Reset the pi"""
    # led off
    GPIO.output(LED_PIN, GPIO.LOW)
    # Release resource(To clean up at the end of your script)
    GPIO.cleanup(LED_PIN)


def main() -> None:
    """Run the main blink program"""
    setup()
    try:
        while True:
            loop()
    except KeyboardInterrupt:
        destroy()


if __name__ == "__main__":
    main()
