#include <wiringPi.h> // Include WiringPi library!
#include <stdbool.h>   // Used for exit()
#include <stdint.h>   // Used for exit()
#include "constants.h"

typedef struct {
    uint8_t data[5];
} reading;

bool read_bit(int pin) {
    return pulseInNS(pin, HIGH, PULSE_TIMEOUT) > PULSE_THRESHOLD;
}

void req_measure(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(18000);
    digitalWrite(pin, HIGH);
    delayMicroseconds(30);
    pinMode(pin, INPUT);
}

uint64_t get_temp(int pin) {
    uint64_t r = 0;
    pulseInNS(pin, HIGH, PULSE_TIMEOUT);
    for (int i = 0; i < 40; i++) {
        r = (r << 1) | read_bit(pin);
    }
    return r;
}

uint64_t get_measure(int pin) {
    req_measure(pin);
    return get_temp(pin);
}
