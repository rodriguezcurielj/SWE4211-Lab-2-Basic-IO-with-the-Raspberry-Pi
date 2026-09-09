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
#if (GPIO_LIB_USED==LGPIO)
#include "LgpioGPIO.h"
#include <lgpio.h>
#include <time.h>
#include <pthread.h>
#include "GenericThreadInfo.h"
#include <sys/syscall.h>
#include <unistd.h>

using namespace std;

namespace SWE4211RPi {

void mylgpioInterrupt0(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt1(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt2(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt3(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt4(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt5(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt6(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt7(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt8(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt9(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt10(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt11(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt12(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt13(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt14(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt15(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt16(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt17(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt18(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt19(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt20(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt21(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt22(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt23(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt24(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt25(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt26(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt27(int e, lgGpioAlert_p evt, void *data);
void mylgpioInterrupt28(int e, lgGpioAlert_p evt, void *data);

/**
 * This static variable keeps track of the GPIO instances as they are instantiated.
 * It is used to ensure that we can reference a given instance based upon
 * the GPIO number of the given pin.
 */
static std::mutex cmtx;

/**
 * This will initialize the initialzed variable to 0 until a constructor is invoked.
 */
int LgpioGPIO::initialized = -1;


void LgpioGPIO::freeGPIOInstance(LgpioGPIO& instance)
{
	instance.instanceCount--;
	if (instance.instanceCount==0)
	{
		int number = instance.number;
		delete gpio_instances[number];
		gpio_instances[number]=nullptr;
	}
}

/**
 * This is the constructor for an instance of a GPIO class.
 * @param number This is the Raspberry Pi GPIO Number.
 * @param dir This is the direction for the GPIO pin, either input or
 * output.
 * @param val This is the default value for the GPIO pin.
 */
LgpioGPIO::LgpioGPIO(int number, DIRECTION dir, VALUE val) {
	std::unique_lock<std::mutex> lck(cmtx);
	// If this is the first time that a constructor has been invoked, initialize
	// the lgpio library devices and setup the pointers.
	if (LgpioGPIO::initialized == -1) {
		for (int index = 0; index < NUMBER_OF_GPIO_PINS; index++) {
			gpio_instances[index] = NULL;
		}
		LgpioGPIO::initialized = lgGpiochipOpen(0); // open /dev/gpiochip0
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
		if (val == GPIO::VALUE::GPIO_LOW) {
			lgGpioClaimOutput(LgpioGPIO::initialized, 0, number, 0); /* initial level 0 */
		} else {
			lgGpioClaimOutput(LgpioGPIO::initialized, 0, number, 1); /* initial level 1 */
		}
	} else {
		lgGpioClaimInput(LgpioGPIO::initialized, 0, number);
	}
}

LgpioGPIO::~LgpioGPIO() {
	// Remove the reference to this instance before it goes away.
	gpio_instances[number] = NULL;
}

/**
 * This method will drive an output pin to the appropriate value.
 * @param This is the value that is to be written to the pin.
 */
void LgpioGPIO::setValue(GPIO::VALUE val) {
	if (val == GPIO::VALUE::GPIO_LOW) {
		lgGpioWrite(LgpioGPIO::initialized, number, 0);
	} else {
		lgGpioWrite(LgpioGPIO::initialized, number, 1);
	}
}

/**
 * This method will return the current state of this GPIO pin.
 * @return The return will be high or low depending on the state of the pin.
 */
GPIO::VALUE LgpioGPIO::getValue() {
	if ( lgGpioRead(LgpioGPIO::initialized, number)==1) {
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
void LgpioGPIO::enableEdgeInterrupt(EDGE edgeType) {
	// This will determine what type of response is expected.
	int responsiveEdges;

	if (edgeType == GPIO_RISING) {
		responsiveEdges = LG_RISING_EDGE;
	} else if (edgeType == GPIO_FALLING) {
		responsiveEdges = LG_FALLING_EDGE;
	} else if (edgeType == GPIO_BOTH) {
		responsiveEdges = LG_BOTH_EDGES;
	} else {
		responsiveEdges = 0;
	}


	// The following code is ugly, but it sets up the method to be invoked
	// when an edge transition occurs. There is a much cleaner way of doing
	// this with table entries and function pointers.
	switch (this->number) {
	case 0:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt0, NULL);
		break;
	case 1:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt1, NULL);
		break;
	case 2:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt2, NULL);
		break;
	case 3:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt3, NULL);
		break;
	case 4:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt4, NULL);
		break;
	case 5:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt5, NULL);
		break;
	case 6:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt6, NULL);
		break;
	case 7:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt7, NULL);
		break;
	case 8:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt8, NULL);
		break;
	case 9:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt9, NULL);
		break;
	case 10:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt10, NULL);
		break;
	case 11:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt11, NULL);
		break;
	case 12:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt12, NULL);
		break;
	case 13:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt13, NULL);
		break;
	case 14:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt14, NULL);
		break;
	case 15:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt15, NULL);
		break;
	case 16:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt16, NULL);
		break;
	case 17:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt17, NULL);
		break;
	case 18:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt18, NULL);
		break;
	case 19:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt19, NULL);
		break;
	case 20:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt20, NULL);
		break;
	case 21:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt21, NULL);
		break;
	case 22:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt22, NULL);
		break;
	case 23:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt23, NULL);
		break;
	case 24:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt24, NULL);
		break;
	case 25:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt25, NULL);
		break;
	case 26:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt26, NULL);
		break;
	case 27:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt27, NULL);
		break;
	case 28:
		lgGpioClaimAlert(LgpioGPIO::initialized, 0, responsiveEdges, this->number, -1);
		lgGpioSetAlertsFunc(LgpioGPIO::initialized, this->number, &mylgpioInterrupt28, NULL);
		break;
	}
}



/**
 * This method will be invoked if an interrupt is enabled whenever a
 * transition in pin value occurs.
 */
void LgpioGPIO::handleInterruptDrivenGPIOPin() {
	if (lgGpioRead(LgpioGPIO::initialized, number)==1) {
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
 * This method will be invoked if an interrupt is enabled whenever a
 * transition in pin value occurs.
 */
void LgpioGPIO::handleInterruptDrivenGPIOPin2(lgGpioAlert_p evt) {
	if (evt[0].report.level==1) {
		isrRisingTimestamp.tv_sec = evt[0].report.timestamp / 1000000000L;
		isrRisingTimestamp.tv_nsec = evt[0].report.timestamp % 1000000000L;
	} else {
		isrFallingTimestamp.tv_sec = evt[0].report.timestamp / 1000000000L;
		isrFallingTimestamp.tv_nsec = evt[0].report.timestamp % 1000000000L;
	}
	// Notify the condition variable for a blocked call.
	cv.notify_one();
	// Notify the condition variable for the callback.
	ccv.notify_one();
}



/**
 * This method, and the following methods, will be invoked as ISR's when an edge transitions if edge transitions are enabled.
 */
void mylgpioInterrupt0(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[0]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[0]==NULL)
	{
		GPIO::threadInstances[0] = new GenericThreadInfo("GPIO 0", syscall(SYS_gettid));
	}
}

void mylgpioInterrupt1(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[1]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[1]==NULL)
	{
		GPIO::threadInstances[1] = new GenericThreadInfo("GPIO 1", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt2(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[2]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[2]==NULL)
	{
		GPIO::threadInstances[2] = new GenericThreadInfo("GPIO 2", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt3(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[3]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[3]==NULL)
	{GPIO::
		threadInstances[3] = new GenericThreadInfo("GPIO 3", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt4(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[4]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[4]==NULL)
	{
		GPIO::threadInstances[4] = new GenericThreadInfo("GPIO 4", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt5(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[5]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[5]==NULL)
	{
		GPIO::threadInstances[5] = new GenericThreadInfo("GPIO 5", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt6(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[6]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[6]==NULL)
	{
		GPIO::threadInstances[6] = new GenericThreadInfo("GPIO 6", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt7(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[7]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[7]==NULL)
	{
		GPIO::threadInstances[7] = new GenericThreadInfo("GPIO 7", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt8(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[8]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[8]==NULL)
	{
		GPIO::threadInstances[8] = new GenericThreadInfo("GPIO 8", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt9(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[9]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[9]==NULL)
	{
		GPIO::threadInstances[9] = new GenericThreadInfo("GPIO 9", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt10(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[10]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[10]==NULL)
	{
		GPIO::threadInstances[10] = new GenericThreadInfo("GPIO 10", syscall(SYS_gettid));
	}


}
void mylgpioInterrupt11(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[11]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[11]==NULL)
	{
		GPIO::threadInstances[11] = new GenericThreadInfo("GPIO 11", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt12(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[12]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[12]==NULL)
	{
		GPIO::threadInstances[12] = new GenericThreadInfo("GPIO 12", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt13(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[13]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[13]==NULL)
	{
		GPIO::threadInstances[13] = new GenericThreadInfo("GPIO 13", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt14(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[14]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[14]==NULL)
	{
		GPIO::threadInstances[14] = new GenericThreadInfo("GPIO 14", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt15(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[15]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[15]==NULL)
	{
		GPIO::threadInstances[15] = new GenericThreadInfo("GPIO 15", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt16(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[16]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[16]==NULL)
	{
		GPIO::threadInstances[16] = new GenericThreadInfo("GPIO 16", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt17(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[17]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[17]==NULL)
	{
		GPIO::threadInstances[17] = new GenericThreadInfo("GPIO 17", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt18(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[18]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[18]==NULL)
	{
		GPIO::threadInstances[18] = new GenericThreadInfo("GPIO 18", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt19(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[19]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[19]==NULL)
	{
		GPIO::threadInstances[19] = new GenericThreadInfo("GPIO 19", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt20(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[20]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[20]==NULL)
	{
		GPIO::threadInstances[20] = new GenericThreadInfo("GPIO 20", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt21(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[21]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[21]==NULL)
	{
		GPIO::threadInstances[21] = new GenericThreadInfo("GPIO 21", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt22(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[22]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[22]==NULL)
	{
		GPIO::threadInstances[22] = new GenericThreadInfo("GPIO 22", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt23(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[23]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[23]==NULL)
	{
		GPIO::threadInstances[23] = new GenericThreadInfo("GPIO 23", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt24(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[24]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[24]==NULL)
	{
		GPIO::threadInstances[24] = new GenericThreadInfo("GPIO 24", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt25(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[25]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[25]==NULL)
	{
		GPIO::threadInstances[25] = new GenericThreadInfo("GPIO 25", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt26(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[26]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[26]==NULL)
	{
		GPIO::threadInstances[26] = new GenericThreadInfo("GPIO 26", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt27(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[27]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[27]==NULL)
	{
		GPIO::threadInstances[27] = new GenericThreadInfo("GPIO 27", syscall(SYS_gettid));
	}

}
void mylgpioInterrupt28(int e, lgGpioAlert_p evt, void *data) {
	GPIO::gpio_instances[28]->handleInterruptDrivenGPIOPin();
	if (GPIO::threadInstances[28]==NULL)
	{
		GPIO::threadInstances[28] = new GenericThreadInfo("GPIO 28", syscall(SYS_gettid));
	}

}


} /* namespace exploringRPi */

#endif
