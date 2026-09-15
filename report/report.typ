= ECSE303 #box(width: 1fr)[#align(right)[Lab \#1]]

== Lab Partners: Trevor Swan, Justin, Trevor Nichols #box(width: 1fr)[#align(right)[Group \#11]]

=== Assignment Statement and Objective(s):
The purpose of this assignment is to get familiar with the Raspberry Pi, including SSH access, GPIO interaction, code compilation, and interacting with the filesystem. This assignment aims to teach basic interaction with the Pi's GPIO pins through the command line and programmatically through both C (compiled) and Python (interpreted).

=== Approach and Techniques:

- To determine the pin layout of our Pi, we  used the `gpio readall` command through the CLI
```
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
 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
```
- We used the `gpio` builtin command for our CLI-based GPIO interaction
- We used the `rpi-lgpio` library for our Python-based GPIO interaction
- We linked against the `wiringPi` library for our C-based GPIO interaction
- The circuit we built was based off of a simple LED setup shown in lecture
- This setup requires a somewhat weak resistor, LED, and two wires to hook up to the pi
- We will use signal handlers in the different languages to handle interrupts to ensure resources do not leak

=== Measurements and Results:

- Following the pin layout above, we wired up the circuit to GPIO pin 25
- The `gpio` command could enable and disable the connected LED via `gpio write 0/1`
- Using the makefile, we ran `make` and observed the output c file at `blinker_c` in the `src` directory
- Running both `sudo python blinker.py` and `sudo ./blinker_c` from `src`, we observed:
  - The LED blinked on and off repeatedly, staying on for 500 milliseconds before turning off for 500 milliseconds, looping like this forever before we killed the process

=== Conclusions:

- We completed the lab using
  - A Red LED
  - A 100 kOhm resistor
  - A male-female wire from ground to the breadboard
  - A male-female wire from GPIO 25 to one end of the resistor
- We expected the LED to blink on and off as we ran the code, and running it with sudo worked as expected

=== Notes:

- CLI
  - We set GPIO 25 to output with `gpio mode 25 out`
  - We could toggle the pin using `gpio write 25 0/1`
- You can run `gpio` without being behind a `sudo`
  - This is perhaps because `gpio` is a root command
  - `ls -lh $(which gpio)` returned: `-rwsr-xr-x 1 root root 42K Jul 25  2024 /usr/local/bin/gpio`
- Our user processes with python and C both required `sudo` to function
  - Without sudo, the C process said: `wiringPiSetup: Unable to open /dev/mem or /dev/gpiomem: Permission denied.
      Aborting your program because if it can not access the GPIO
      hardware then it most certianly won't work
      Try running with sudo?`
- We used nix to orchestrate the builds of the c, python, and shell script
  - This outputs the file to `result/bin` and gives executables that you can run to `sudo`
