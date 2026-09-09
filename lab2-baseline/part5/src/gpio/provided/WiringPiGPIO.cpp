/**
 * @file GPIO.cpp
 * @author  Walter Schilling (schilling@msoe.edu)
 * @version 1.0
 *
 * @section LICENSE
 *
 *
 * This code is developed as part of the MSOE SWE4211 Real Time Systems course,
 * but can be freely used by others.
 *
 * SWE4211 Real Time Systems is a required course for students studying the
 * discipline of software engineering.
 *
 * This Software is provided under the License on an "AS IS" basis and
 * without warranties of any kind concerning the Software, including
 * without limitation merchantability, fitness for a particular purpose,
 * absence of defects or errors, accuracy, and non-infringement of
 * intellectual property rights other than copyright. This disclaimer
 * of warranty is an essential part of the License and a condition for
 * the grant of any rights to this Software.
 *
 * @section DESCRIPTION
 *
 * This file contains a set of GPIO wrapper classes for the Raspberry Pi.
 * This class allows a user
 * of the Raspberry PI to readily interface with the wiringPi libraries in an
 * object oriented fashion in C++, invoking callbacks, and writing C++ using
 * objects.
 */

#include "GPIOcfg.h"
#include "WiringPiGPIO.h"

#define WIRING_PI (1)
#define LGPIO (2)


#include <wiringPi.h>
#include <time.h>
#include <pthread.h>
#include "GenericThreadInfo.h"
#include <sys/syscall.h>
#include <unistd.h>

using namespace std;

namespace SWE4211RPi {

/**
 * This static variable keeps track of the GPIO instances as they are instantiated.
 * It is used to ensure that we can reference a given instance based upon
 * the GPIO number of the given pin.
 */
//static WiringPiGPIO* gpio_instances[NUMBER_OF_GPIO_PINS+1];

static std::mutex cmtx;

/**
 * This will initialize the initialzed variable to 0 until a constructor is invoked.
 */
int WiringPiGPIO::initialized = 0;

/**
 * This is the constructor for an instance of a GPIO class.
 * @param number This is the Raspberry Pi GPIO Number.
 * @param dir This is the direction for the GPIO pin, either input or
 * output.
 * @param val This is the default value for the GPIO pin.
 */
WiringPiGPIO::WiringPiGPIO(int number, DIRECTION dir, VALUE val) {
	std::unique_lock<std::mutex> lck(cmtx);
	// If this is the first time that a constructor has been invoked, initialize
	// the wiringPi devices and setup the pointers.
	if (WiringPiGPIO::initialized == 0) {
		for (int index = 0; index < NUMBER_OF_GPIO_PINS; index++) {
			gpio_instances[index] = NULL;
		}
		wiringPiSetupGpio();
		WiringPiGPIO::initialized++;
	}

	// Set the number accordingly, the reference to this class,
	// and callback to be nothing for now.
	this->number = number;
	gpio_instances[number] = this;
	methodCallback = NULL;

	this->instanceCount++;

	// Initialize the timestamps accordingly.
	isrRisingTimestamp.tv_nsec = 0;
	isrRisingTimestamp.tv_sec = 0;
	isrFallingTimestamp.tv_nsec = 0;
	isrFallingTimestamp.tv_sec = 0;

	// Based upon the GPIO direction, setup the pin accordingly.
	if (dir == GPIO::DIRECTION::GPIO_OUT) {
		pinMode(number, OUTPUT);
		if (val == GPIO::VALUE::GPIO_LOW) {
			digitalWrite(number, LOW);
		} else {
			digitalWrite(number, HIGH);
		}
	} else {
		pinMode(number, INPUT);
	}
	
}

WiringPiGPIO::~WiringPiGPIO() {
	// Remove the reference to this instance before it goes away.
	gpio_instances[number] = NULL;
}

/**
 * This method will drive an output pin to the appropriate value.
 * @param This is the value that is to be written to the pin.
 */
void WiringPiGPIO::setValue(GPIO::VALUE val) {
	if (val == GPIO::VALUE::GPIO_LOW) {
		digitalWrite(number, LOW);
	} else {
		digitalWrite(number, HIGH);
	}
}

/**
 * This method will return the current state of this GPIO pin.
 * @return The return will be high or low depending on the state of the pin.
 */
GPIO::VALUE WiringPiGPIO::getValue() {
	if (digitalRead(number) == HIGH) {
		return GPIO::GPIO_HIGH;
	} else {
		return GPIO::GPIO::GPIO_LOW;
	}
}

/**
 * This method will enable an interrupt to be processed based upon edges.
 * In doing this, callbacks can be made based upon the GPIO pin changing
 * state and methods can block for IO purposes.
 * @param edgeType This is the edge type that is to be processed as an
 * interrupt.
 */
void WiringPiGPIO::enableEdgeInterrupt(EDGE edgeType) {
	// This will determine what type of response is expected.
	int responsiveEdges;

	if (edgeType == GPIO_RISING) {
		responsiveEdges = INT_EDGE_RISING;
	} else if (edgeType == GPIO_FALLING) {
		responsiveEdges = INT_EDGE_FALLING;
	} else if (edgeType == GPIO_BOTH) {
		responsiveEdges = INT_EDGE_BOTH;
	} else {
		responsiveEdges = INT_EDGE_SETUP;
	}

	// The following code is ugly, but it sets up the method to be invoked
	// when an edge transition occurs. There is a much cleaner way of doing
	// this with table entries and function pointers.
	switch (this->number) {
	case 0:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt0);
		break;
	case 1:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt1);
		break;
	case 2:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt2);
		break;
	case 3:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt3);
		break;
	case 4:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt4);
		break;
	case 5:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt5);
		break;
	case 6:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt6);
		break;
	case 7:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt7);
		break;
	case 8:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt8);
		break;
	case 9:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt9);
		break;
	case 10:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt10);
		break;
	case 11:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt11);
		break;
	case 12:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt12);
		break;
	case 13:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt13);
		break;
	case 14:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt14);
		break;
	case 15:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt15);
		break;
	case 16:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt16);
		break;
	case 17:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt17);
		break;
	case 18:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt18);
		break;
	case 19:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt19);
		break;
	case 20:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt20);
		break;
	case 21:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt21);
		break;
	case 22:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt22);
		break;
	case 23:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt23);
		break;
	case 24:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt24);
		break;
	case 25:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt25);
		break;
	case 26:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt26);
		break;
	case 27:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt27);
		break;
	case 28:
		wiringPiISR(this->number, responsiveEdges, &myInterrupt28);
		break;
	}
}



/**
 * This method will be invoked if an interrupt is enabled whenever a
 * transition in pin value occurs.
 */
void WiringPiGPIO::handleInterruptDrivenGPIOPin() {
	if (digitalRead(number) == HIGH) {
		// Update the timestamp for the interrupt.
		clock_gettime(CLOCK_REALTIME, &isrRisingTimestamp);
	} else {
		// Update the timestamp for the interrupt.
		clock_gettime(CLOCK_REALTIME, &isrFallingTimestamp);
	}
	// Notify the condition variable for a blocked call.
	cv.notify_one();
	// Notify the condition variable for the callback.
	ccv.notify_one();
}

/**
 * This method, and the following methods, will be invoked as ISR's when an edge transitions if edge transitions are enabled.
 */
void myInterrupt0(void) {
	GPIO::gpio_instances[0]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[0]==NULL)
	{
		GPIO::threadInstances[0] = new GenericThreadInfo("GPIO 0", syscall(SYS_gettid));
	}
}

void myInterrupt1(void) {
	GPIO::gpio_instances[1]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[1]==NULL)
	{
		GPIO::threadInstances[1] = new GenericThreadInfo("GPIO 1", syscall(SYS_gettid));
	}

}
void myInterrupt2(void) {
	GPIO::gpio_instances[2]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[2]==NULL)
	{
		GPIO::threadInstances[2] = new GenericThreadInfo("GPIO 2", syscall(SYS_gettid));
	}

}
void myInterrupt3(void) {
	GPIO::gpio_instances[3]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[3]==NULL)
	{GPIO::
		threadInstances[3] = new GenericThreadInfo("GPIO 3", syscall(SYS_gettid));
	}


}
void myInterrupt4(void) {
	GPIO::gpio_instances[4]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[4]==NULL)
	{
		GPIO::threadInstances[4] = new GenericThreadInfo("GPIO 4", syscall(SYS_gettid));
	}


}
void myInterrupt5(void) {
	GPIO::gpio_instances[5]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[5]==NULL)
	{
		GPIO::threadInstances[5] = new GenericThreadInfo("GPIO 5", syscall(SYS_gettid));
	}


}
void myInterrupt6(void) {
	GPIO::gpio_instances[6]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[6]==NULL)
	{
		GPIO::threadInstances[6] = new GenericThreadInfo("GPIO 6", syscall(SYS_gettid));
	}


}
void myInterrupt7(void) {
	GPIO::gpio_instances[7]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[7]==NULL)
	{
		GPIO::threadInstances[7] = new GenericThreadInfo("GPIO 7", syscall(SYS_gettid));
	}


}
void myInterrupt8(void) {
	GPIO::gpio_instances[8]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[8]==NULL)
	{
		GPIO::threadInstances[8] = new GenericThreadInfo("GPIO 8", syscall(SYS_gettid));
	}


}
void myInterrupt9(void) {
	GPIO::gpio_instances[9]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[9]==NULL)
	{
		GPIO::threadInstances[9] = new GenericThreadInfo("GPIO 9", syscall(SYS_gettid));
	}


}
void myInterrupt10(void) {
	GPIO::gpio_instances[10]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[10]==NULL)
	{
		GPIO::threadInstances[10] = new GenericThreadInfo("GPIO 10", syscall(SYS_gettid));
	}


}
void myInterrupt11(void) {
	GPIO::gpio_instances[11]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[11]==NULL)
	{
		GPIO::threadInstances[11] = new GenericThreadInfo("GPIO 11", syscall(SYS_gettid));
	}

}
void myInterrupt12(void) {
	GPIO::gpio_instances[12]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[12]==NULL)
	{
		GPIO::threadInstances[12] = new GenericThreadInfo("GPIO 12", syscall(SYS_gettid));
	}

}
void myInterrupt13(void) {
	GPIO::gpio_instances[13]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[13]==NULL)
	{
		GPIO::threadInstances[13] = new GenericThreadInfo("GPIO 13", syscall(SYS_gettid));
	}

}
void myInterrupt14(void) {
	GPIO::gpio_instances[14]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[14]==NULL)
	{
		GPIO::threadInstances[14] = new GenericThreadInfo("GPIO 14", syscall(SYS_gettid));
	}

}
void myInterrupt15(void) {
	GPIO::gpio_instances[15]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[15]==NULL)
	{
		GPIO::threadInstances[15] = new GenericThreadInfo("GPIO 15", syscall(SYS_gettid));
	}

}
void myInterrupt16(void) {
	GPIO::gpio_instances[16]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[16]==NULL)
	{
		GPIO::threadInstances[16] = new GenericThreadInfo("GPIO 16", syscall(SYS_gettid));
	}

}
void myInterrupt17(void) {
	GPIO::gpio_instances[17]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[17]==NULL)
	{
		GPIO::threadInstances[17] = new GenericThreadInfo("GPIO 17", syscall(SYS_gettid));
	}

}
void myInterrupt18(void) {
	GPIO::gpio_instances[18]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[18]==NULL)
	{
		GPIO::threadInstances[18] = new GenericThreadInfo("GPIO 18", syscall(SYS_gettid));
	}

}
void myInterrupt19(void) {
	GPIO::gpio_instances[19]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[19]==NULL)
	{
		GPIO::threadInstances[19] = new GenericThreadInfo("GPIO 19", syscall(SYS_gettid));
	}

}
void myInterrupt20(void) {
	GPIO::gpio_instances[20]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[20]==NULL)
	{
		GPIO::threadInstances[20] = new GenericThreadInfo("GPIO 20", syscall(SYS_gettid));
	}

}
void myInterrupt21(void) {
	GPIO::gpio_instances[21]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[21]==NULL)
	{
		GPIO::threadInstances[21] = new GenericThreadInfo("GPIO 21", syscall(SYS_gettid));
	}

}
void myInterrupt22(void) {
	GPIO::gpio_instances[22]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[22]==NULL)
	{
		GPIO::threadInstances[22] = new GenericThreadInfo("GPIO 22", syscall(SYS_gettid));
	}

}
void myInterrupt23(void) {
	GPIO::gpio_instances[23]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[23]==NULL)
	{
		GPIO::threadInstances[23] = new GenericThreadInfo("GPIO 23", syscall(SYS_gettid));
	}

}
void myInterrupt24(void) {
	GPIO::gpio_instances[24]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[24]==NULL)
	{
		GPIO::threadInstances[24] = new GenericThreadInfo("GPIO 24", syscall(SYS_gettid));
	}

}
void myInterrupt25(void) {
	GPIO::gpio_instances[25]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[25]==NULL)
	{
		GPIO::threadInstances[25] = new GenericThreadInfo("GPIO 25", syscall(SYS_gettid));
	}

}
void myInterrupt26(void) {
	GPIO::gpio_instances[26]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[26]==NULL)
	{
		GPIO::threadInstances[26] = new GenericThreadInfo("GPIO 26", syscall(SYS_gettid));
	}

}
void myInterrupt27(void) {
	GPIO::gpio_instances[27]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[27]==NULL)
	{
		GPIO::threadInstances[27] = new GenericThreadInfo("GPIO 27", syscall(SYS_gettid));
	}

}
void myInterrupt28(void) {
	GPIO::gpio_instances[28]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[28]==NULL)
	{
		GPIO::threadInstances[28] = new GenericThreadInfo("GPIO 28", syscall(SYS_gettid));
	}

}


} /* namespace exploringRPi */
