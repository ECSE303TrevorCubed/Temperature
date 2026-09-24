#include "poll.h"

#include <assert.h>
#include <signal.h>  // Required for signal handling
#include <stdbool.h> // Used for exit()
#include <stdio.h>
#include <stdlib.h>   // Used for exit()
#include <wiringPi.h> // Include WiringPi library!

#include "constants.h"

static void req_measure(int pin) {
  // Start signal: pull down for 18 ms, pull up for 40 us
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(18);
  digitalWrite(pin, HIGH);
  delayMicroseconds(40);
  pinMode(pin, INPUT);
}

// Returns false when the checksum is invalid or read timed out
bool read_dht11_polling(int pin, Data *data) {
  assert(data && "Data out pointer invalid");
  uint8_t raw[5] = {0, 0, 0, 0, 0};
  uint8_t last_state = HIGH;
  int delay_counter_us = 0;
  uint8_t bits_recv = 0; // Counts to 40

  req_measure(pin);
  for (int i = 0; i < MAX_TIMINGS; ++i) {
    delay_counter_us = 0;
    while (digitalRead(pin) == last_state) {
      delayMicroseconds(1);
      if (++delay_counter_us == 255) {
        break; // Exceeded delay timeout
      }
    }
    last_state = digitalRead(pin);

    // If timeout occurred, stop reading
    if (delay_counter_us == 255) {
      break;
    }

    // Ignore the first 3 transitions (sensor initial response)
    if (i < 4) {
      continue;
    }

    // Data bits on falling edges (even index)
    if (i % 2 == 0) {
      raw[bits_recv / 8] <<= 1;
      // Midpoint between 0-bit count (~12-15) and 1-bit count (~35-45)
      if (delay_counter_us > POLL_COUNTER_THRESHOLD) {
        raw[bits_recv / 8] |= 1;
      }
      bits_recv++;
      if (bits_recv >= TOTAL_BITS_PER_READ) {
        break; // All 40 bits successfully received; prevents raw[5] buffer overflow
      }
    }
  }

  // Verify 40 bits received and checksum matches
  if (bits_recv < TOTAL_BITS_PER_READ) {
    fprintf(stderr, "Did not receive 40 bits: actual %d\n", bits_recv);
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

  fprintf(stderr, "Checksum was wrong: actual %d != expected %d (raw: %d %d %d %d %d)\n",
         sum, raw[4], raw[0], raw[1], raw[2], raw[3], raw[4]);
  return false;
}
