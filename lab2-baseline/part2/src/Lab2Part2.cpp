/**
 * @file
 * @author  Walter Schilling (schilling@msoe.edu)
 * @version 2.0
 *
 * @section LICENSE This code is distributed to students in order that they complete lab 2 on latency and performance.
 *
 *
 * @section DESCRIPTION
 *
 * This program will blink a light at a given rate.  Because of it's usage of the real time extensions of the OS, it must be run as the super user.
 */

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "GPIO.h"
#include <unistd.h>
#include <sys/mman.h>


using namespace SWE4211RPi;
using namespace std;

/**
 * This program will blink a light on a given GPIO on and off.
 * The rate and number of blinks are set by the user.
 */
int main(int argc, char* argv[])
{
	// Check to determine if the command line usage is correct or not.
	if (argc != 4)
	{
		cerr << "Usage: " << argv[0] << " <GPIO PIN> <Flashes Per Second> <Time To Flash>";
		exit(-1);
	}

	// Determine the period and the blink count.
	int GPIOPin = atoi(argv[1]);
	int flashesPerSecond =  atoi(argv[2]);
	int executionTime = atoi(argv[3]);

	// Instantiate a new instance of a GPIO port.
	GPIO& outGPIO = GPIO::getInstance(GPIOPin, GPIO::GPIO_OUT);

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

	int loopCount = executionTime * flashesPerSecond;
	int period = 1000000 / flashesPerSecond;
	int onTime = period / 2;
	int offTime = period - (period / 2);

	// Loop over the data, turning things on and off as is necessary.
	for (int index = 0; index < loopCount; index++)
	{
		// Turn the light on.
		outGPIO.setValue(GPIO::GPIO_LOW);

		// Cause the thread to sleep for a given period of time.
		usleep(onTime);

		// Turn the pin off.
		outGPIO.setValue(GPIO::GPIO_HIGH);

		// Go to sleep for a given period of time.
		usleep(offTime);
	}
	GPIO::freeInstance(outGPIO);
}
