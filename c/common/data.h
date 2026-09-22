#ifndef DHT_DATA_H_
#define DHT_DATA_H_

#include <stdint.h>

typedef struct Data {
  uint8_t relative_hum_int;
  uint8_t relative_hum_dec;
  uint8_t temperature_int;
  uint8_t temperature_dec;
  uint8_t checksum;
} Data;

#endif
