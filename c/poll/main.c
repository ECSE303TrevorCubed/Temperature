// #include <assert.h>
// #include <signal.h>  // Required for signal handling
// #include <stdbool.h> // Used for exit()
// #include <stdio.h>
// #include <stdlib.h>   // Used for exit()
// #include <wiringPi.h> // Include WiringPi library!

// #include "constants.h"
// #include "data.h"
// #include "log.h"
// #include "poll.h"
// #include "prio.h"

// // Signal handler
// static void handleSignal(int sig) {
//   (void)sig;
//   // Clean up GPIO states before exiting
//   digitalWrite(DHT11_PIN, LOW);
//   pinMode(DHT11_PIN, INPUT); // Reset pin back to input for safety

//   // Terminate the process cleanly
//   exit(0);
// }

// int main(void) {
//   // Setup stuff:
//   signal(SIGINT, handleSignal);
//   FILE *log = fopen("temper_poll.log", "a"); // append
//   if (!log) {
//     printf("Failed to open log file!\n");
//     return 1;
//   }

//   // Set high priority for approaching rt scheduling
//   if (!try_set_prio(99)) {
//     printf("Failed to set priority! Try running with sudo?\n");
//     return 1;
//   }

//   if (wiringPiSetup() == -1) {
//     exit(1);
//   }

//   Data data;
//   while (true) {
//     if (!read_dht11_polling(DHT11_PIN, &data)) {
//       log_fail(log);
//       log_fail(stderr);
//     } else {
//       log_data(log, data);
//       log_data(stdout, data);
//     }

//     delay(LOOP_TIMEOUT_MS);
//   }

//   fclose(log);
//   return 0;
// }

// gcc -Os dht1_polling.c -lwiringPi -o dht1_polling

#include <wiringPi.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define MAXTIMINGS	85
#define DHTPIN		25
int dht11_dat[5] = { 0, 0, 0, 0, 0 };

void read_dht11_dat()
{
	uint8_t laststate	= HIGH;
	uint8_t counter		= 0;
	uint8_t j		= 0, i;
	float	f; 						// fahrenheit
 
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
 
	// pull pin down for 18 milliseconds
	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 18 );
	// pull it up for 40 microseconds
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 40 );
	// prepare to read the pin
	pinMode( DHTPIN, INPUT );
 
	// detect change and read data
	for ( i = 0; i < MAXTIMINGS; i++ )
	{
		counter = 0;
		while ( digitalRead( DHTPIN ) == laststate )
		{
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 )
			{
				break;
			}
		}
		laststate = digitalRead( DHTPIN );
 
		if ( counter == 255 )
			break;
 
		// ignore first 3 transitions
		if ( (i >= 4) && (i % 2 == 0) )
		{
			// shove each bit into the storage bytes
			dht11_dat[j / 8] <<= 1;
			if ( counter > 16 )
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}

	printf("relative_hum_int = %d\n", dht11_dat[0]);
    printf("relative_hum_dec = %d\n", dht11_dat[1]);
    printf("temperature_int = %d\n", dht11_dat[2]);
    printf("temperature_dec = %d\n", dht11_dat[3]);
    printf("checksum = %d\n", dht11_dat[4]);
 
	// verify checksum with reading data.

	//print humidity and temperature

	//print time

	//write humidity and temperature into file
		
}
 
int main()
{ 
	if ( wiringPiSetup() == -1 )
		exit( 1 );
 
	while ( 1 )
	{
		read_dht11_dat();
		// wait 1sec to refresh
		delay( 1000 ); 
	}
 
	return(0);
}
