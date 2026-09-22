#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#include "constants.h"
#include "data.h"
#include "interrupt.h"
#include "log.h"
#include "prio.h"

// Signal handler
static void handleSignal(int sig) {
  (void)sig;
  // Clean up GPIO states before exiting
  digitalWrite(DHT11_PIN, LOW);
  pinMode(DHT11_PIN, INPUT);

  // Terminate the process cleanly
  exit(0);
}

int main(void) {
    // Setup stuff:
  signal(SIGINT, handleSignal);
  FILE *log = fopen("temper_interr.log", "a"); // append
  if (!log) {
    printf("Failed to open log file!\n");
    return 1;
  }
  
  // Set high priority for approaching rt scheduling
  if (!try_set_prio(99)) {
    printf("Failed to set priority! Try running with sudo?\n");
    return 1;
  }

  if (wiringPiSetup() == -1) {
    exit(1);
  }

  Data data;
  while (true) {
    if (!read_dht11_interrupt(DHT11_PIN, &data)) {
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
