#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <wiringPi.h>

#include "constants.h"
#include "data.h"
#include "lcd.h"
#include "log.h"
#include "poll.h"
#include "prio.h"
#include "threshold.h"

// Signal handler for clean exit
static void handleSignal(int sig) {
  (void)sig;
  digitalWrite(DHT11_PIN, LOW);
  pinMode(DHT11_PIN, INPUT);
  exit(0);
}

int main(void) {
  signal(SIGINT, handleSignal);

  // Set high priority for approaching rt scheduling
  if (!try_set_prio(99)) {
    printf("Failed to set priority! Try running with sudo?\n");
    return 1;
  }

  if (wiringPiSetup() == -1) {
    exit(1);
  }

  lcd_t lcd;
  if (!lcd_init(&lcd)) {
    fprintf(stderr, "Failed to initialize I2C LCD\n");
    return 1;
  }

  FILE *log = fopen("temper_display.log", "a"); // append

  Data data;
  while (true) {
    if (!read_dht11_polling(DHT11_PIN, &data)) {
      log_fail(log);
      log_fail(stderr);
    } else {
      log_data(log, data);
      log_data(stderr, data);
      record_data(data);
      averages_t current_averages = average_readings();

      lcd_clear(lcd); // So we don't have visual artifacts
      lcd_set_cursor(lcd, 0, 0);
      lcd_printf(lcd, "Temp: %.1f C", current_averages.temperature);
      lcd_set_cursor(lcd, 0, 1);
      lcd_printf(lcd, "Humidity: %.1f %%", current_averages.humidity);
    }

    delay(LOOP_TIMEOUT_MS);
  }
}
