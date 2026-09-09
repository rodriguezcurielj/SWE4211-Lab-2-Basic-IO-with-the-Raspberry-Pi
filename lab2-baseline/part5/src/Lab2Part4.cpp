/**
* @file
* @author  Lihui Mei
* @version 3.0
* @section DESCRIPTION
*
* This program will allow the user to control a light.
* The light will be controlled by pressing and releasing a pushbutton.
* Turns LED D2 on when pushbutton S1 is pressed, off when released.
*/

#include "GPIO.h"
#include <iostream>
#include <unistd.h>
#include <wiringPi.h>
#include "WiringPiGPIO.h"
#include <sys/mman.h>

using namespace SWE4211RPi;
using namespace std;

int main(int argc, char *argv[]) {
	// Check to determine if the command line usage is correct or not.
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0]
			<< " <led_bcm_pin> <button_bcm_pin> <sample_period_in_ms>" << std::endl;
		exit(-1);
	}

	// Print out your name and your lab partner's name to the console at the start of the program.
	cout << "Lab 2: Lihui Mei and Jose Rodriguez Curiel" << endl;

	// Setup the operating thread to be a real time thread.
	struct sched_param p;
	int maxPriority = sched_get_priority_max(SCHED_FIFO);
	if (maxPriority == -1) {
		std::cerr << "Failed to query max scheduler priority" << std::endl;
		exit(-1);
	}
	p.__sched_priority = (maxPriority > 70) ? 70 : (maxPriority - 10);

	// Lock memory and prefault the stack. A page fault mid-run stalls a transition,
	// and it'll show up as an outlier in your data.
	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (sched_setscheduler(0, SCHED_FIFO, &p) != 0) {
		printf("Failed to set the scheduler\n");
		exit(-1);
	}

	// Determine the GPIO pin numbers and sleep time.  Argument order matches
	// the python script: LED pin, button pin, sample period in ms.
	int gpioOutPin = atoi(argv[1]);
	int gpioInPin = atoi(argv[2]);
	int sleepPeriodinms = atoi(argv[3]);

	// Instantiate a new instance of a GPIO port to act as a GPIO output port.
	// Start with the LED off (active low -> HIGH means off).
	WiringPiGPIO outPin(gpioOutPin, GPIO::DIRECTION::GPIO_OUT, GPIO::VALUE::GPIO_HIGH);

	// Instantiate a new instance of a GPIO port to act as a GPIO input port.
	WiringPiGPIO inPin(gpioInPin, GPIO::DIRECTION::GPIO_IN);

	// The WiringPiGPIO wrapper does not enable the internal pull-up resistor
	// when configuring a pin as input, so the button pin floats and reads
	// unreliably.  We are not permitted to modify WiringPiGPIO.cpp (it's a
	// provided file), so we enable the pull-up here directly using the raw
	// wiringPi call.  By this point wiringPiSetupGpio() has already run
	// inside the WiringPiGPIO constructor, so this is safe to call.
	pullUpDnControl(gpioInPin, PUD_UP);

	// Loop forever.  (Well, until Ctrl-C is pressed.)
	while (1 == 1) {
		// Pull-up configuration: button released -> pin reads HIGH,
		// button pressed -> pin reads LOW.
		// LED is active low: LOW turns it on, HIGH turns it off.
		if (inPin.getValue() == GPIO::GPIO_LOW) {
			// Button pressed -> turn LED on.
			outPin.setValue(GPIO::GPIO_LOW);
		} else {
			// Button released -> turn LED off.
			outPin.setValue(GPIO::GPIO_HIGH);
		}

		if (sleepPeriodinms != 0) {
			usleep(sleepPeriodinms * 1000);
		}
	}

	return 0;
}
