#include "poll.h"

#include <assert.h>
#include <signal.h>  // Required for signal handling
#include <stdbool.h> // Used for exit()
#include <stdio.h>
#include <stdlib.h>   // Used for exit()
#include <wiringPi.h> // Include WiringPi library!

#include "constants.h"

static void req_measure(int pin) {
  // Start signal: host pulls low for at least 18ms (20ms is recommended)
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(20);

  // Pull up for ~30us then switch to input with pull-up resistor
  digitalWrite(pin, HIGH);
  delayMicroseconds(30);
  pinMode(pin, INPUT);
  pullUpDnControl(pin, PUD_UP);
}

// Returns false when the checksum is invalid or read timed out
bool read_dht11_polling(int pin, Data *data) {
  assert(data && "Data out pointer invalid");
  uint8_t raw[5] = {0, 0, 0, 0, 0};
  uint8_t last_state = HIGH;
  uint8_t bits_recv = 0; // Counts to 40

  req_measure(pin);
  for (int i = 0; i < MAX_TIMINGS; ++i) {
    unsigned int start_us = micros();
    while (digitalRead(pin) == last_state) {
      if ((micros() - start_us) > 255) {
        break; // Exceeded delay timeout
      }
    }
    unsigned int duration_us = micros() - start_us;
    last_state = digitalRead(pin);

    // If a timeout occurred, stop reading
    if (duration_us >= 255) {
      break;
    }

    // Ignore the first 3 transitions (sensor initial response)
    if (i < 4) {
      continue;
    }

    // Data bits on falling edges (even index)
    if (i % 2 == 0) {
      if (bits_recv < TOTAL_BITS_PER_READ) {
        raw[bits_recv / 8] <<= 1;
        if (duration_us > PULSE_WIDTH_THRESHOLD_US) {
          raw[bits_recv / 8] |= 1;
        }
        bits_recv++;
        if (bits_recv == TOTAL_BITS_PER_READ) {
          break; // All 40 bits successfully received
        }
      }
    }
  }

  // Verify 40 bits received and checksum matches
  if (bits_recv < TOTAL_BITS_PER_READ) {
    printf("Did not receive 40 bits: actual %d\n", bits_recv);
    return false;
  }
  uint8_t sum = raw[0] + raw[1] + raw[2] + raw[3];
  if (sum == raw[4]) {
    *data = (Data){
        .relative_hum_int = raw[0],
        .relative_hum_dec = raw[1],
        .temperature_int = raw[2],
        .temperature_dec = raw[3],
        .checksum = raw[4],
    };
    return true;
  }

  printf("Checksum was wrong: actual %d\n", sum);
  return false;
}
