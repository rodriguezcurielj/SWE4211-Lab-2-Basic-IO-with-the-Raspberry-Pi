/**
 * @file
 * @author  <Place your name here>
 * @version 2.0
  * @section DESCRIPTION
 *
 * This program will allow the user to control a light.
 * The light will be controlled by pressing and releasing a pushbutton.
 */

#include "GPIO.h"
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>

using namespace SWE4211RPi;
using namespace std;

/**
 * This program will control the LED.  It essentially will turn the LED on if the button is pressed and off if the button is released.
 */
int main(int argc, char *argv[]) {
	// Check to determine if the command line usage is correct or not.
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0]
				<< " <Output GPIO Pin Number> <Input GPIO Pin Number> <Sleep period in milliseconds>";
		exit(-1);
	}
	// Print out your name and your lab partners name to the console at the start of the program.
	// TODO


	// Setup the operating thread to be a real time thread.
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_FIFO) > 70 ? 70 : sched_get_priority_max(SCHED_FIFO) - 10;

	// Lock memory and prefault the stack. A page fault mid-run stalls a transition,
	// and it'll show up as an outlier in your data.
	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (sched_setscheduler(0, SCHED_FIFO, &p) != 0) {
		printf("Failed to set the scheduler\n");
		exit(-1);
	}


	// Determine the GPIO pin numbers and sleep time.
	int gpioOutPin = atoi(argv[1]);
	int gpioInPin = atoi(argv[2]);
	int sleepPeriodinms = atoi(argv[3]);

	// Instantiate a new instance of a GPIO port to act as a GPIO output port.
	// TODO

	// Instantiate a new instance of a GPIO port to act as a GPIO input port.
	// TODO


	// Loop forever.  (Well, until the Ctrl-C is pressed.)
	while (1 == 1) {
		// Read the input pin.  If the pin is low,
		// TODO

			// then turn the light on.
		// TODO
		// else
		// TODO
			// else if the pin is not low, Turn the light off.
		// TODO

		// If the sleep time is not zero, go to sleep for the appropriate amount of time.
		if (sleepPeriodinms!=0)
		{
			// Cause the thread to sleep for a given period of time.
			// TODO
		}
	}
}
