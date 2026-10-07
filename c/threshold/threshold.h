#ifndef TEMPER_THRESH_H_
#define TEMPER_THRESH_H_

#include "data.h"

typedef struct {
  float humidity;
  float temperature;
} averages_t;

// Returns the floating point value of the temperature
void record_data(Data data);
averages_t average_readings(void);

#endif