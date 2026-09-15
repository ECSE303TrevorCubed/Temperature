#include <signal.h>   // Required for signal handling
#include <stdlib.h>   // Used for exit()
#include <wiringPi.h> // Include WiringPi library!

const int ledPin = 26;
const int waitTime = 500;

// Signal handler
void handleSignal(int sig) {
  // Clean up GPIO states before exiting
  digitalWrite(ledPin, LOW);
  pinMode(ledPin, INPUT); // Reset pin back to input for safety

  // Terminate the process cleanly
  exit(0);
}

int main(void) {
  // Setup stuff:
  signal(SIGINT, handleSignal);
  wiringPiSetupGpio();     // Initialize wiringPi -- using Broadcom pin numbers
  pinMode(ledPin, OUTPUT); // Set regular LED as output

  while (true) {
    digitalWrite(ledPin, LOW);
    delay(waitTime);
    digitalWrite(ledPin, HIGH);
    delay(waitTime);
  }

  return 0;
}
