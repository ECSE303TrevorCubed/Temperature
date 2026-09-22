#ifndef TEMPER_POLL_H_
#define TEMPER_POLL_H_

#include <stdbool.h>

#include "data.h"

bool read_dht11_polling(int pin, Data *data);

#endif
