#include <signal.h>  // Required for signal handling
#include <stdbool.h> // Used for exit()
#include <stdio.h>
#include <stdlib.h>   // Used for exit()
#include <wiringPi.h> // Include WiringPi library!

#include "constants.h"
#include "dht11.h"

const int dht11Pin = 26;
const int waitTime = 500;

// Signal handler
void handleSignal(int sig) {
  // Clean up GPIO states before exiting
  digitalWrite(dht11Pin, LOW);
  pinMode(dht11Pin, INPUT); // Reset pin back to input for safety

  // Terminate the process cleanly
  exit(0);
}

int main(void) {
  // Setup stuff:
  signal(SIGINT, handleSignal);
  if (wiringPiSetupGpio() == -1) {
    exit(1);
  }

  const uint64_t measurement = get_measure(dht11Pin);
  const Data data = data_decode(measurement);
  printf("Relative Int = %d\n", data.relative_hum_int);
  printf("Relative Dec = %d\n", data.relative_hum_dec);
  printf("Temperature Int = %d\n", data.temperature_int);
  printf("Temperature Dec = %d\n", data.relative_hum_dec);
  printf("Checksum = %d\n", data.checksum);

  // while (true) {
  //   digitalWrite(dht11Pin, LOW);
  //   delay(waitTime);
  //   digitalWrite(dht11Pin, HIGH);
  //   delay(waitTime);
  // }

  return 0;
}
