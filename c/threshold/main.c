#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#include "constants.h"
#include "data.h"
#include "log.h"
#include "poll.h"
#include "threshold.h"

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
  pinMode(LED_PIN, OUTPUT);

  FILE *log = fopen("temper_threshold.log", "a"); // append

  Data data;
  while (true) {
    if (!read_dht11_polling(DHT11_PIN, &data)) {
      log_fail(log);
      log_fail(stderr);
      continue;
    }

    record_temperature(data.temperature_int, data.temperature_dec);
    float current_average = average_celsius();
    if (current_average > TEMPERATURE_THRESHOLD_CELSIUS) {
      log_data(log, "Temperature exceeded the threshold: %f C",
               current_average);
      log_data(stdout, "Temperature exceeded the threshold: %f C",
               current_average);
      digitalWrite(LED_PIN, HIGH);
    } else {
      log_data(log, "Temperature dropped below the threshold: %f C",
               current_average);
      log_data(stdout, "Temperature dropped below the threshold: %f C",
               current_average);
      digitalWrite(LED_PIN, LOW);
    }

    delay(LOOP_TIMEOUT_MS);
  }

  fclose(log);

  return 0;
}
