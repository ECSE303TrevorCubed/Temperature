#include "interrupt.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>

#include "constants.h"
#include "data.h"

typedef enum State {
  INIT_PULL_LINE_LOW,
  INPUT_JUST_ENABLED,
  HIGH_ACK,
  BIT_READ_RISING,
  READ_COMPLETE,
  ERROR_STATE
} State;

static volatile State current_state = READ_COMPLETE;
static volatile int current_reading_bit_idx = 0;
static volatile int micros_previous_rising_edge = 0;
static volatile int bits_rcvd[TOTAL_BITS_PER_READ];
static volatile bool read_ready = false;

static bool isr_registered = false;
static int sensor_pin;

/*
 * ISR for reading the sensor. There are several states which this ISR accounts
 * for.
 */
static void sensor_read_isr(void) {
  switch (current_state) {
  case INIT_PULL_LINE_LOW:
    if (read_ready) {
      current_reading_bit_idx = 0;
      micros_previous_rising_edge = 0;
      current_state = INPUT_JUST_ENABLED;
      pinMode(sensor_pin, INPUT);
      pullUpDnControl(sensor_pin, PUD_UP);
    }
    break;
  case INPUT_JUST_ENABLED:
    current_state = HIGH_ACK;
    break;
  case HIGH_ACK:
    current_state = BIT_READ_RISING;
    break;
  case BIT_READ_RISING: {
    int current_us = micros(); // Get the time when this edge occurred

    // For the first bit, initialize rising edge timestamp as baseline.
    if (current_reading_bit_idx == 0) {
      micros_previous_rising_edge = current_us;
      ++current_reading_bit_idx;
      break;
    }

    // Time difference between consecutive rising edges
    int delta = current_us - micros_previous_rising_edge;
    micros_previous_rising_edge = current_us;

    // A complete bit cycle (50us low pre-bit + data high):
    // '0' bit: ~50us low + ~28us high = ~78us total
    // '1' bit: ~50us low + ~70us high = ~120us total
    int prev_bit_high_time = delta - PRE_BIT_DELAY;

    // Check for abnormal cycle timing (sensor disconnected, missed edges)
    if (delta > 250 || delta < 30) {
      current_state = ERROR_STATE;
      break;
    }

    // Distinguish 0 vs 1 based on pulse high duration
    if (prev_bit_high_time <= PULSE_WIDTH_THRESHOLD_US) {
      bits_rcvd[current_reading_bit_idx - 1] = 0;
    } else {
      bits_rcvd[current_reading_bit_idx - 1] = 1;
    }

    ++current_reading_bit_idx; // Account for this bit's high time
    if (current_reading_bit_idx >= TOTAL_BITS_PER_READ) {
      current_state = READ_COMPLETE;
      // Wait for pulse discrimination window (~40us after rising edge)
      int elapsed = micros() - current_us;
      if (elapsed < 40) {
        delayMicroseconds(40 - elapsed);
      }
      if (digitalRead(sensor_pin) == LOW) {
        bits_rcvd[current_reading_bit_idx - 1] = 0;
      } else {
        bits_rcvd[current_reading_bit_idx - 1] = 1;
      }
    }
    break;
  }

  default:
    break;
  }
}

/*
 * Decodes an 8-bit integer starting at bits[offset].
 */
static uint8_t extract_byte_at_offset(const volatile int *bits, int offset) {
  int acc = 0;
  for (int bit_idx = 0; bit_idx < BITS_PER_BYTE; ++bit_idx) {
    acc |= (bits[offset + 7 - bit_idx] & 1) << bit_idx;
  }
  return (uint8_t)acc;
}

bool read_dht11_interrupt(int pin, Data *data) {
  assert(data && "Data out pointer must not be NULL");
  sensor_pin = pin;

  // Register the ISR if not already done
  if (!isr_registered) {
    if (wiringPiISR(sensor_pin, INT_EDGE_RISING, sensor_read_isr) < 0) {
      return false;
    }
    isr_registered = true;
  }

  // Clear buffers
  memset((void *)bits_rcvd, 0, sizeof(bits_rcvd));
  read_ready = false;

  // Initiate read: pull line low for 20ms to signal DHT11
  current_state = INIT_PULL_LINE_LOW;
  pinMode(sensor_pin, OUTPUT);
  digitalWrite(sensor_pin, LOW);
  delay(20);

  // Switch to input with pull-up resistor to release the line
  pinMode(sensor_pin, INPUT);
  pullUpDnControl(sensor_pin, PUD_UP);

  // Wait for DHT11 to acknowledge by pulling the line low (nominally 20-40us)
  int wait_us = 0;
  while (digitalRead(sensor_pin) == HIGH && wait_us < 100) {
    delayMicroseconds(1);
    wait_us++;
  }
  if (digitalRead(sensor_pin) == HIGH) {
    // Sensor did not pull the line low
    current_state = ERROR_STATE;
    return false;
  }

  // The DHT11 is now asserting its 80us LOW response.
  // The next rising edge on the bus is guaranteed to be the DHT11's 80us HIGH
  // ACK pulse.
  current_reading_bit_idx = 0;
  micros_previous_rising_edge = 0;
  current_state = HIGH_ACK;
  read_ready = true;

  // Wait for ISR state machine completion or timeout (max 100ms)
  int timeout_us = 0;
  while (current_state != READ_COMPLETE && current_state != ERROR_STATE &&
         timeout_us < 100000) {
    delayMicroseconds(500);
    timeout_us += 500;
  }

  // Ensure pin is back to input with pull-up and reset state
  pinMode(sensor_pin, INPUT);
  pullUpDnControl(sensor_pin, PUD_UP);

  if (current_state != READ_COMPLETE) {
    current_state = READ_COMPLETE;
    printf("Read did not complete\n");
    return false;
  }
  current_state = READ_COMPLETE;

  // Extract readings and checksum
  uint8_t humid_int = extract_byte_at_offset(bits_rcvd, 0);
  uint8_t humid_dec = extract_byte_at_offset(bits_rcvd, 8);
  uint8_t temp_int = extract_byte_at_offset(bits_rcvd, 16);
  uint8_t temp_dec = extract_byte_at_offset(bits_rcvd, 24);
  uint8_t checksum_read = extract_byte_at_offset(bits_rcvd, 32);

  uint8_t sum = humid_int + humid_dec + temp_int + temp_dec;
  if (checksum_read != sum) {
    printf("Checksum was wrong: actual %d\n", sum);
    return false;
  }

  *data = (Data){
      .relative_hum_dec = humid_dec,
      .relative_hum_int = humid_int,
      .temperature_int = temp_int,
      .temperature_dec = temp_dec,
      .checksum = checksum_read,
  };

  return true;
}
