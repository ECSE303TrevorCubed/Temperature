#include "threshold.h"

#include <stddef.h>

#include "constants.h"

static float temperature_readings[MAXIMUM_TEMPERATURE_READINGS] = {0};
static size_t index = 0;
static bool full = false;

static float parse_from_parts(int dec_part, int frac_part) {
  float lhs = (float)dec_part;
  if (frac_part == 0) {
    return lhs;
  }

  float divisor = 1.0;
  int temp = frac_part;

  while (temp > 0) {
    divisor *= 10.0;
    temp /= 10;
  }
  return lhs + (float)frac_part / divisor;
}

void record_temperature(int dec_part, int frac_part) {
  temperature_readings[index] = parse_from_parts(dec_part, frac_part);
  index = (index + 1) % MAXIMUM_TEMPERATURE_READINGS;
  if (index == 0)
    full = true;
}

float average_celsius(void) {
  const int count = full ? MAXIMUM_TEMPERATURE_READINGS : index;
  if (count == 0)
    return 0.0f;
  float sum = 0.0f;
  for (int i = 0; i < count; ++i) {
    sum += temperature_readings[i];
  }
  return sum / count;
}
