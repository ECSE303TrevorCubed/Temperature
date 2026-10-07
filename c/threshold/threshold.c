#include "threshold.h"

#include <stdbool.h>
#include <stddef.h>

#include "constants.h"

static float temperature_readings[MAXIMUM_BUFFERED_READINGS] = {0};
static float humidity_readings[MAXIMUM_BUFFERED_READINGS] = {0};
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

void record_data(Data data) {
  temperature_readings[index] =
      parse_from_parts(data.temperature_int, data.temperature_dec);
  humidity_readings[index] =
      parse_from_parts(data.relative_hum_int, data.relative_hum_dec);
  index = (index + 1) % MAXIMUM_BUFFERED_READINGS;
  if (index == 0)
    full = true;
}

averages_t average_readings(void) {
  averages_t avgs = {0};
  const int count = full ? MAXIMUM_BUFFERED_READINGS : index;
  if (count == 0)
    return avgs;
  float hum_sum = 0.0f, temp_sum = 0.0f;
  for (int i = 0; i < count; ++i) {
    hum_sum += temperature_readings[i];
    temp_sum += humidity_readings[i];
  }
  avgs.humidity = hum_sum / count;
  avgs.temperature = temp_sum / count;
  return avgs;
}
