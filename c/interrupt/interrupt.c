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
static volatile int measured_bit_high_time[TOTAL_BITS_PER_READ];

static bool isr_registered = false;
static int sensor_pin;

/*
 * ISR for reading the sensor. There are several states which this ISR accounts for.
 */
static void sensor_read_isr(void) {
  switch (current_state) {
    case INIT_PULL_LINE_LOW:
      if (read_ready) {
        current_reading_bit_idx = 0;
        micros_previous_rising_edge = 0;
        current_state = INPUT_JUST_ENABLED;
        digitalWrite(sensor_pin, HIGH);
        pinMode(sensor_pin, INPUT);
      }
      break;

      // Temporary, single-use state. It serves to handle
		// the case where the sensor pin is set to an input
		// in the INIT_PULL_LINE_LOW state, triggering an
		// unwanted interrupt.
    case INPUT_JUST_ENABLED:
      current_state = HIGH_ACK;
      break;

      // Entered at the beginning of the high response
    case HIGH_ACK:
      current_state = BIT_READ_RISING;
      break;

    case BIT_READ_RISING: {
        // Get the time when this edge occurred
      int current_us = micros();

      // For the first bit, initialize rising edge timestamp as baseline.
      if (current_reading_bit_idx == 0) {
        micros_previous_rising_edge = current_us;
        ++current_reading_bit_idx;
        break;
      }

      // High time for previous bit = (total interval between rising edges) - pre-bit low time (50us)
      int prev_bit_high_time = (current_us - micros_previous_rising_edge) - PRE_BIT_DELAY;
      micros_previous_rising_edge = current_us;

      // Distinguish 0 vs 1 based on pulse high duration
      if (prev_bit_high_time <= (MAX_TIME_FOR_ZERO_BIT_US + MAX_TIME_BUFFER)) {
        bits_rcvd[current_reading_bit_idx - 1] = 0;
      } else if (prev_bit_high_time <= (MAX_TIME_FOR_ONE_BIT_US + MAX_TIME_BUFFER)) {
        bits_rcvd[current_reading_bit_idx - 1] = 1;
      } else {
        current_state = ERROR_STATE;
      }

      measured_bit_high_time[current_reading_bit_idx - 1] = prev_bit_high_time;
      ++current_reading_bit_idx;

      // When the final bit (index 39) is reached, there is no subsequent rising edge.
      // Delay 40us and poll the line: if still HIGH it's a 1, if LOW it's a 0.
      if (current_reading_bit_idx >= TOTAL_BITS_PER_READ) {
        current_state = READ_COMPLETE;
        delayMicroseconds(40);
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
static int extract_byte_at_offset(const volatile int *bits, int offset) {
  int acc = 0;
  for (int bit_idx = 0; bit_idx < BITS_PER_BYTE; ++bit_idx) {
    acc |= (bits[offset + 7 - bit_idx] & 1) << bit_idx;
  }
  return acc;
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
  memset((void *)bits_rcvd, -1, sizeof(bits_rcvd));
  memset((void *)measured_bit_high_time, -1, sizeof(measured_bit_high_time));
  read_ready = false;

  // Initiate read: pull line low for 20ms to signal DHT11
  current_state = INIT_PULL_LINE_LOW;
  pinMode(sensor_pin, OUTPUT);
  digitalWrite(sensor_pin, LOW);
  delay(20);

  // Set line ready and trigger ISR transition to INPUT
  read_ready = true;
  sensor_read_isr();

  // Wait for ISR state machine completion or timeout (max 100ms)
  int timeout_us = 0;
  while (current_state != READ_COMPLETE && current_state != ERROR_STATE &&
         timeout_us < 100000) {
    delayMicroseconds(500);
    timeout_us += 500;
  }

  // Ensure pin is back to input with pull-up
  pinMode(sensor_pin, INPUT);
  pullUpDnControl(sensor_pin, PUD_UP);

  if (current_state != READ_COMPLETE) {
    return false;
  }

  // Extract readings and checksum
  uint8_t humid_int = (uint8_t)extract_byte_at_offset(bits_rcvd, 0);
  uint8_t humid_dec = (uint8_t)extract_byte_at_offset(bits_rcvd, 8);
  uint8_t temp_int = (uint8_t)extract_byte_at_offset(bits_rcvd, 16);
  uint8_t temp_dec = (uint8_t)extract_byte_at_offset(bits_rcvd, 24);
  uint8_t checksum_read = (uint8_t)extract_byte_at_offset(bits_rcvd, 32);

  uint8_t checksum_calc = humid_int + humid_dec + temp_int + temp_dec;
  if (checksum_read != checksum_calc) {
    return false;
  }

  data->relative_hum_int = humid_int;
  data->relative_hum_dec = humid_dec;
  data->temperature_int = temp_int;
  data->temperature_dec = temp_dec;
  data->checksum = checksum_read;

  return true;
}
