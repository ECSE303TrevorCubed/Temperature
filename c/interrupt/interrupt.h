#ifndef TEMPER_INTERRUPT_H_
#define TEMPER_INTERRUPT_H_

#include <stddef.h>
#include <stdint.h>

typedef enum State {
  INIT_PULL_LINE_LOW,
  INPUT_JUST_ENABLED,
  HIGH_ACK,
  BIT_READ_RISING,
  READ_COMPLETE,
  ERROR_STATE
} State;

void initiate_read(void);
void sensor_read_isr(void);
void setup_gpio(void);
void release_gpio(void);
int extract_val(volatile uint8_t *bits_rcvd, size_t offset);
int gen_checksum(volatile uint8_t *bits_rcvd);

#endif