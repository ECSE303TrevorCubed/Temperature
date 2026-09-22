#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#include "constants.h"
#include "dht11.h"

// Signal handler for clean exit
static void handleSignal(int sig) {
  (void)sig;
  digitalWrite(LED_PIN, LOW);
  pinMode(LED_PIN, INPUT);
  digitalWrite(DHT11_PIN, LOW);
  pinMode(DHT11_PIN, INPUT);
  exit(0);
}

int main(void) {
  signal(SIGINT, handleSignal);
  if (wiringPiSetup() == -1) {
    exit(1);
  }

  FILE* log = fopen("temper_threshold.log", "a"); // append
  
  fclose(log);

  return 0;
}
