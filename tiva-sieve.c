/****************
Embedded Sieve of Eratosthenes
https://github.com/szczys/embedded-sieve-of-eratosthenes

Find prime numbers by eliminating multiples from a list.

The list is an array of 32-bit registers. Each bit
represents a number, with the index of the bit being the
number itself.

The arry is stored in SRAM, allowing the use of the
ARM core Bit-Banding functionality to modify bits
without a read, modify, write activity.
****************/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

//Array to serve as the number list for hunting primes
#define MAX_ARRAY 10000
uint32_t sieve[MAX_ARRAY/32];


void initClock(void) {
	//Start the system clock
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
}

void initIO(void) {
    //Enable PortF
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //LED Pins as outputs
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

void initUART(void) {
	//Enable PORTA
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	//Enable UART
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	//Configure IO
	ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	//Setup UART clock
	UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
	//Set baud
	UARTStdioConfig(0, 115200, 16000000);
}

int main(void)
{

    //Initialization
	ROM_FPULazyStackingEnable();
    initClock();
	initIO();
	initUART();

	while(1)
	{
		UARTprintf("\n\nHunting for Primes:\n");

		//Set up the array

		sieve[0] = 0xFFFFFFFC;
		for (unsigned int i=1; i<MAX_ARRAY/32; i++) {
			sieve[i] = 0xFFFFFFFF;
		}

	
		int prime_counter = 0;
		int prime_found = 0;
		while (1) {
			//increment counter (0 isn't prime anyway) and check for overflow
			if (++prime_found >= MAX_ARRAY) { break; }

			//read the current index to see if it is a 1
			uint32_t arrayIndex = prime_found/32;		
			//prime found
			// if not, increment and try again
			//if (sieve[arrayIndex] & (1<<(prime_found%32)))
			if (HWREGBITW(&sieve[arrayIndex],prime_found%32))
			{
				// print out the prime
				UARTprintf("%d ",prime_found);
				// Increment the counter
				++prime_counter;

				// remove all multiples of this prime by setting zero in the bit-packed array
				//TODO: track how long this takes
			
				uint32_t indexMultiplier = prime_found;
				uint32_t multipleIndex = prime_found * indexMultiplier++;
				while(multipleIndex < MAX_ARRAY) {
					//UARTprintf("NotPrime: %d",multipleIndex);
					arrayIndex = multipleIndex/32;
					HWREGBITW(&sieve[arrayIndex],multipleIndex%32) = 0;
					multipleIndex = prime_found * indexMultiplier++;
				}
			}
		}
	UARTprintf("\n\nFound %d primes between 0 and %d", prime_counter, MAX_ARRAY);

    

		//Red
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);		//Turn on LED
		ROM_SysCtlDelay( (ROM_SysCtlClockGet()/(3*1000))*1000) ; 	//delay ~1s
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);				//Turn off LED

		//Blue
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);		//Turn on LED
		ROM_SysCtlDelay( (ROM_SysCtlClockGet()/(3*1000))*1000) ; 	//delay ~1s
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);				//Turn off LED

		//Green
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);		//Turn on LED
		ROM_SysCtlDelay( (ROM_SysCtlClockGet()/(3*1000))*1000) ; 	//delay ~1s
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);				//Turn off LED
		
    }
}
