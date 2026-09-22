#ifndef TEMPER_LOG_H_
#define TEMPER_LOG_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct Data;

void log_fail(FILE *f);
void log_data(FILE *f, struct Data data);

#endif
