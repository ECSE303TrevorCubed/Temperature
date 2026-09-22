#include "poll.h"

#include <assert.h>
#include <signal.h>  // Required for signal handling
#include <stdbool.h> // Used for exit()
#include <stdio.h>
#include <stdlib.h>   // Used for exit()
#include <wiringPi.h> // Include WiringPi library!

#include "constants.h"

static void req_measure(int pin) {
  // Start signal
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(18); // Pull down for 18 ms
  digitalWrite(pin, HIGH);
  delayMicroseconds(40); // Pull up for 40 microseconds
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
      if (++delay_counter_us == 255)
        return false; // Exceeded delay timeout
    }
    last_state = digitalRead(pin);
    if (i < 4)
      continue; // ignore first 3 transitions

    // Data bits on falling edges (even)
    if (i % 2 == 0) {
      raw[bits_recv / 8] <<= 1;
      if (delay_counter_us > PULSE_WIDTH_THRESHOLD_US) {
        raw[bits_recv / 8] |= 1;
      }
      bits_recv++;
    }
  }

  // Verify 40 bits received and checksum matches
  if (bits_recv < 40)
    return false;
  uint8_t sum = raw[0] + raw[1] + raw[2] + raw[3];
  if (sum == raw[4]) {
    data->relative_hum_int = raw[0];
    data->relative_hum_dec = raw[1];
    data->temperature_int = raw[2];
    data->temperature_dec = raw[3];
    data->checksum = raw[4];
    return true;
  }

  return false;
}
