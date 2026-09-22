#include <stdint.h>

uint64_t get_measure(int pin);

typedef struct {
  uint8_t relative_hum_int;
  uint8_t relative_hum_dec;
  uint8_t temperature_int;
  uint8_t temperature_dec;
  uint8_t checksum;
} Data;

Data data_decode(uint64_t raw);
