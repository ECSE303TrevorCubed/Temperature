#ifndef TEMPER_DHT11_H_
#define TEMPER_DHT11_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  uint8_t relative_hum_int;
  uint8_t relative_hum_dec;
  uint8_t temperature_int;
  uint8_t temperature_dec;
  uint8_t checksum;
} Data;

// Returns false when the checksum is invalid
bool read_dht11(int pin, Data* data);
void log_fail(FILE* f);
void log_data(FILE* f, Data data);

#endif
