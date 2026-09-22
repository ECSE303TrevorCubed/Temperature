#ifndef TEMPER_INTERRUPT_H_
#define TEMPER_INTERRUPT_H_

#include <stdbool.h>

#include "data.h"

bool read_dht11_interrupt(int pin, Data *data);

#endif