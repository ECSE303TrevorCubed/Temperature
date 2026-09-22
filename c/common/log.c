#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "data.h"
#include "log.h"

static void log_time(FILE *f) {
  assert(f && "File pointer is not valid");
  time_t now = time(NULL);
  char buf[128];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
  fprintf(f, "[%s] ", buf);
}

void log_fail(FILE *f) {
  log_time(f);
  fprintf(f, "Failed to read DHT11 data\n");
}

void log_data(FILE *f, Data data) {
  log_time(f);
  fprintf(f, "Temp: %d.%d C, Humidity: %d.%d %%, Checksum: %02X\n",
          data.temperature_int, data.temperature_dec, data.relative_hum_int,
          data.relative_hum_dec, data.checksum);
}
