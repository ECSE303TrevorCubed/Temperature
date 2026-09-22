#include <signal.h>  // Required for signal handling
#include <stdbool.h> // Used for exit()
#include <stdio.h>
#include <stdlib.h>   // Used for exit()
#include <wiringPi.h> // Include WiringPi library!

#include "constants.h"
#include "dht11.h"

// Signal handler
static void handleSignal(int sig) {
  // Clean up GPIO states before exiting
  digitalWrite(DHT11_PIN, LOW);
  pinMode(DHT11_PIN, INPUT); // Reset pin back to input for safety

  // Terminate the process cleanly
  exit(0);
}

int main(void) {
  // Setup stuff:
  signal(SIGINT, handleSignal);
  if (wiringPiSetup() == -1) {
    exit(1);
  }

  FILE* log = fopen("temper_poll.log", "a"); // append

  Data data;
  while (true) {
      if (!read_dht11(DHT11_PIN, &data)) {
          log_fail(log);
          log_fail(stderr);
      } else {
          log_data(log, data);
          log_data(stdout, data);
      }
      
      delay(LOOP_TIMEOUT_MS);
  }

  fclose(log);
  return 0;
}
