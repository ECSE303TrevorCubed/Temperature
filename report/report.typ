#show link: set text(fill: blue)

= ECSE303 #box(width: 1fr)[#align(right)[Lab \#2]]

== Lab Partners: Trevor Swan, Justin Fossum, Trevor Nichols #box(width: 1fr)[#align(right)[Group \#11]]

=== Assignment Statement and Objective(s):
The objective of this assignment is to interface the Raspberry Pi with a Hitachi HD44780 16x2 Liquid Crystal Display (LCD) over the I2C serial bus using a PCF8574 8-bit I/O Expander. This part builds upon previous single-wire DHT11 temperature and humidity acquisition methods, specifically using the sensor polling and moving average logic to drive data collection and display. The main goal of this lab is to use the low-level API provided by wiringPi to effectively interface with the HD44780 and dynamically display live readings from the environment.

==== Summary of Contents
- *Part 4:* Implemented LCD actuation based on sensor readings. Used a circular buffer that calculates a moving average of the previous 10 temperature and humidity readings, displaying the current average to the LCD's screen via an I2C-driven HD44780 display controller.

=== Methodology and Steps:
==== Hardware Configuration
The circuit connects both the DHT11 sensor (from previous parts) and an LCD to the Raspberry Pi:
- *DHT11 Sensor*:
  - Data Pin: Connected to GPIO 25 (`#define DHT11_PIN 25`, physical pin 37) with a 10 kOhm pull-up resistor to the 3.3V power rail.
  - Power and Ground: Connected to 3.3V (physical pin 1) and Ground (physical pin 39).
- *LCD with PCF8574 I2C Backpack*:
  - VCC: Connected to the 5V power rail (physical pin 2) to supply adequate voltage for LCD contrast and backlight LED.
  - GND: Connected to Ground (physical pin 6).
  - SDA (I2C Data): Connected to GPIO 2 (`SDA.1`, physical pin 3, wPi 8).
  - SCL (I2C Clock): Connected to GPIO 3 (`SCL.1`, physical pin 5, wPi 9).
  - Contrast Adjustment: The potentiometer on the backpack was calibrated to ensure character visibility (fully clockwise worked well).

#image("hardware_configuration.png", width: 80%)

The I2C connection was confirmed prior to testing. This required editing the kernel's I2C module with the command: `raspi-config`

#image("raspiconfig.png", width: 80%)

Once the system was rebooted, as reported int the notes, `i2cdetect -y 1` could be used to verify that the bus was operational, which is also where we discovered the address of `0x27`.

==== Part 4: LCD_Based Display

1. I2C Bus & PCF8574 Register Mapping
- The HD44780 requires multiple parallel lines (4–8 data bits, RS, RW, EN). The PCF8574 acts as a middle man, utilizing `/dev/i2c-1` at address 0x27
- Bits written over I2C map directly to control and data lines:
  - Bit 0 (`PIN_RS`): Register Select (`0` for commands, `1` for DDRAM data)
  - Bit 2 (`PIN_EN`): Enable line
  - Bit 3 (`BACKLIGHT_BIT`): Powers the transistor controlling the LCD backlight (kept HIGH on all transfers)
  - Bits 4–7: Connect to HD44780 high nibble data lines ($D_4 - D_7$, not shown below)

#image("pin_fns_of_interest.png", width: 80%)

2. 4-Bit Bus Nibble Protocol & Timing
- We studied the timing employed at this open source #link("https://github.com/arduino-libraries/LiquidCrystal")[GitHub Repository] for precise timings
- The data transmitted must be carefully timed and split into nibbles (4-bits) with the most significant nibble first, followed by the remaining lower bits of the total byte
  - `lcd_write_nibble(fd, nibble, mode)` masks the top 4 bits, attaches `mode` and `BACKLIGHT_BIT`, and drives `PIN_EN` HIGH
  - `PIN_EN` is then cleared, followed by `delayMicroseconds(50)` to satisfy instruction settling time of 37 us
  - `lcd_send()` sequences high and low nibbles to transmit complete commands (`lcd_command`) and characters (`lcd_write`)
  - `lcd_clear()` and `lcd_home()` require extended delays with the spec sheet listing them as extremely slow operations

#image("instructions_of_interest.png", width: 80%)

3. HD44780 Initialization Sequence
- `lcd_init()` implements the initialization by instruction flowchart from the Hitachi HD44780U datasheet:
  - Initial power-on settling delay of 50 ms (`delay(50)`)
  - Three consecutive `0x30` resets with delays (4.5 ms, 4.5 ms, 150 us) to establish reliable synchronization regardless of prior power state
    - Note that the diagram shows `0b000011`. This is equivalent to `0x30` as the first nibble is only transmitted, and `0x3 == 0b11`
  - Transitions to 4-bit mode via `0x20` nibble
  - Enables display without cursor or blink (`LCD_DISPLAYCONTROL`)
  - Clears display DDRAM (`LCD_CLEARDISPLAY`)
  - Sets entry mode to auto-increment cursor left-to-right

#image("init_flowchart.png", width: 80%)

This initialization logic can also be found #link("https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp#L120C5-L133C23")[here].

4. Cursor Addressing and Text Formatting
- The LCD is not contiguous, with the top row address starting at `0x00` and the bottom row starting at `0x40`
- `lcd_set_cursor(lcd, row, col)` sends command `LCD_SETDDRAMADDR | (col + row_offset)`
- `lcd_printf()` wraps `vsnprintf` to format numerical output before streaming characters via `lcd_puts()`

#image("display_addressing.png", width: 80%)

5. Sensor Integration & Moving Average Smoothing
- DHT11 samples are collected similarly to Part 3 of this lab with 1000ms intervals using polling
- Data is converted into its floating point representation based on the integer and decimal parts (`parse_from_parts`) and both the humidity and temperature parsed data are stored in global circular buffers
- `average_readings()` computes the moving average across the buffer
  - This is different from the previous report as it now returns a struct of averages (both the humidity and temperature)
- Each new valid data point logs to a file (`temper_display.log`), prints to standard out, and displays the updated averages to the LCD, formatted as:
  - Row 0: `Temp: %.1f C`
  - Row 1: `Humidity: %.1f %%`

=== Results and Observations:
==== Part 4 Results
The LCD driver and moving average routine working nicely together and reliably presented stable temperature and humidity readings to the screen, updated every second.

===== Contrast Calibration & Hardware Observations
During initial testing, we observed that the backlight turned on but that the characters were not visible. This was adjusted by switching to the 5V power rail and rotating the potentiometer on the back of the LCD until the printed characters could observed. Changing the potentiometer while keeping it on the 3.3V rail did change the appearance of the characters, but not meaningfully enough to make out the display.

===== Display Refresh Behavior
Calling `lcd_clear()` every second clears the previous text to prevent visual artifacts from consecutive writes to the display from overlapping. The issue with this approach is that `lcd_clear()` takes 2 milliseconds to complete, resulting in a subtle flicker being perceptible during updates. It may be advantageous to instead just set the cursor position and hope that the new data overwrites the old data fully, but this may not work in certain environments where the temperature or humidity fluctuates greatly. `lcd_clear()` was the most robust to keep the display synchronized.

===== Sensor Stability
We ran into the same issues with stability on the DHT11 side as discussed in the previous submissions. The benefit to our approach in this lab is that we use a rolling average to keep track of successful reads, dampening the effect missed data points have on the displayed data.

===== Snippet from Log File
```log
[2026-10-07 17:02:51] Temp: 22.2 C, Humidity: 43.0 %, Checksum: 43
[2026-10-07 17:04:10] Failed to read DHT11 data
[2026-10-07 17:04:11] Temp: 21.8 C, Humidity: 43.0 %, Checksum: 48
[2026-10-07 17:04:12] Temp: 21.8 C, Humidity: 43.0 %, Checksum: 48
[2026-10-07 17:04:13] Temp: 21.8 C, Humidity: 43.0 %, Checksum: 48
[2026-10-07 17:04:14] Temp: 21.8 C, Humidity: 43.0 %, Checksum: 48
[2026-10-07 17:04:15] Temp: 21.9 C, Humidity: 43.0 %, Checksum: 49
[2026-10-07 17:04:16] Temp: 22.2 C, Humidity: 43.0 %, Checksum: 43
[2026-10-07 17:04:18] Temp: 22.2 C, Humidity: 43.0 %, Checksum: 43
[2026-10-07 17:04:19] Temp: 22.2 C, Humidity: 43.0 %, Checksum: 43
[2026-10-07 17:04:20] Failed to read DHT11 data
```

These data illustrates consecutive successful readings mixed with the occasional read failures due to checksum mismatches. These mismatches are likely due to scheduler preemption, as discussed in the previous parts's submissions. These failed readings do not impact the state of our moving averages and do not cause a re-render of the LCD's displayed contents.

=== Conclusions:
==== Techniques Learned
- Learned how to interface with an I2C expander (PCF8574) using C and Linux through WiringPi's basic I2C helper library API
- Implemented low-level HD44780 nibble bus sequencing with spec-driven timing delays and custom cursor addressing based on the spec sheet
- Combined real-time priority escalation (for more dependable polling), single-wire sensor reading (from part 1), moving-average data smoothing (from part 3), and visual display formatting into a single embedded app

==== Thoughts on the Assignment
- Communicating over the hardware I2C bus was extremely consistent compared to the single-wire DHT11 polling and interrupt routines explored in the previous parts of the lab
- Implementing the 4-bit protocol manually provided great insight into why libraries provided by Arduino and Espressif developers use specific delays during command sequences and initialization

=== Notes:
- GPIO pin layout:
```txt
+-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
|     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
|   2 |   8 |   SDA.1 | ALT0 | 1 |  3 || 4  |   |      | 5v      |     |     |
|   3 |   9 |   SCL.1 | ALT0 | 1 |  5 || 6  |   |      | 0v      |     |     |
|   4 |   7 | GPIO. 7 |   IN | 0 |  7 || 8  | 1 | ALT5 | TxD     | 15  | 14  |
|     |     |      0v |      |   |  9 || 10 | 1 | ALT5 | RxD     | 16  | 15  |
|  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
|  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
|  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
|     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
|  10 |  12 |    MOSI | ALT0 | 0 | 19 || 20 |   |      | 0v      |     |     |
|   9 |  13 |    MISO | ALT0 | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
|  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
|     |     |      0v |      |   | 25 || 26 | 1 | OUT  | CE1     | 11  | 7   |
|   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
|   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
|   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
|  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
|  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
|  26 |  25 | GPIO.25 |   IN | 1 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
|     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
+-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
```
- We adjusted the contrast of the LCD by turing the potentiometer on the backpack of the LCD
- The LCD must be connected to the 5V power line to work properly
  - Using the 3.3V power did not result in the message being visible
  - Even moving the potentiometer to its highest context, nothing could be seen with 3.3V
- The address to use with the i2c setup command can be found by running: `i2cdetect -y 1`
  - This prints a hex address to use, in this case `0x27`:
```txt
sudo /usr/sbin/i2cdetect -y 1
    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- 27 -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```
- We found the file to use for the LCD by exploring the `/dev` directory on the machine
  - The only i2c related file here was named `i2c-1`, so we pointed our code at `/dev/i2c-1`
- The DHT11 maintained the same wiring as the previous labs: DHT11 Data -> Physical Pin 37 (BCM 25, wPi 25) with 10 kOhm pull-up to 3.3V rail.
- Connections needed for the LCD itself:
  - GND -> Physical Pin 29 (0V)
  - VCC -> Physical Pin 2 (5V)
  - SDA -> Physical Pin 3 (BCM 2, wPi 8), labeled SDA.1
  - SCL -> Physical Pin 5 (BCM 3, wPi 9), labeled SCL.1
- The pins labeled SDA.0 and SCL.0 were not able to be used with the LCD
- After changing the kernel settings with `raspi-config` a reboot is needed to interface with the I2C bus
- `i2cdetect` is available via the `i2c-tools` package, which can be installed via `sudo apt install i2c-tools`
  - The binary is installed to `/usr/sbin`
- We demoed to the TA on 10/8/26
