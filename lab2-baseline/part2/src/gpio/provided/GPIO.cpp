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
#include "GPIO.h"

#define WIRING_PI (1)
#define LGPIO (2)

#if GPIO_LIB_USED==WIRING_PI
#include "WiringPiGPIO.h"
#include <wiringPi.h>
#endif
#if GPIO_LIB_USED==LGPIO
#include <lgpio.h>
#include "LgpioGPIO.h"
#endif

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
GPIO* GPIO::gpio_instances[NUMBER_OF_GPIO_PINS+1];
GenericThreadInfo* GPIO::threadInstances[NUMBER_OF_GPIO_PINS+1];

/**
 * This is a private method that will be used to start a callback handler.
 * @param value This is a void pointer to the GPIO instance.
 */
static void* startCallbackHander(void *gpioInstance);

static std::mutex cmtx;

/**
 * This method will start a thread that will serve as the callback handler thread.
 * @param value This is a void pointer to the GPIO instance.
 */
void* startCallbackHander(void *gpioInstance) {

	GPIO *gpio = static_cast<GPIO*>(gpioInstance);
	gpio->callBackThreadHandler();
	return 0;
}


GPIO& GPIO::getInstance(int number, GPIO::DIRECTION dir, GPIO::VALUE value)
{
	if (GPIO::gpio_instances[number]!=nullptr)
	{
		GPIO::gpio_instances[number]->instanceCount++;
		return *(GPIO::gpio_instances[number]);
	}
#if GPIO_LIB_USED==WIRING_PI
	return *(new WiringPiGPIO(number, dir, value));
#endif
#if GPIO_LIB_USED==LGPIO
	return *(new LgpioGPIO(number, dir, value));
#endif
}

void GPIO::freeInstance(GPIO& instance)
{
#if GPIO_LIB_USED==WIRING_PI

#endif
#if GPIO_LIB_USED==LGPIO
	LgpioGPIO::freeGPIOInstance((LgpioGPIO&)instance);
#endif
}

GPIO::GPIO()
{

}

GPIO::~GPIO()
{

}




/**
 * This method will block the calling thread for up to maxWait ms unless
 * an edge occurs.
 * @param maxWait This is the maximum blocked period in ms.
 * @return 0 if the edge occurred or -1 if a timeout occurred.
 */
int GPIO::waitForEdge(int maxWait) // waits until button is pressed up to a maximum value.
		{
	std::unique_lock<std::mutex> lck(mtx);

	if (std::cv_status::timeout 	== cv.wait_for(lck, std::chrono::milliseconds(maxWait))) {
		// A timeout occurred.  Return -1.
		return -1;
	} else {
		// An edge occurred.
		return 0;
	}
}

/**
 * This method will obtain the timestamp from the last time the pin
 * transitioned high.
 * @return The timestamp of the transition will be returned.
 */
struct timespec GPIO::getRisingISRTimestamp() {
	return isrRisingTimestamp;
}

/**
 * This method will obtain the timestamp from the last time the pin
 * transitioned low.
 * @return The timestamp of the transition will be returned.
 */
struct timespec GPIO::getFallingISRTimestamp() {
	return isrFallingTimestamp;
}


/**
 * This method will set the callback handler.  If this is NULL, no method
 * will be invoked.  However, if it is non-NULL and the class triggers on
 * edges, the method described here will be called when an edge transition
 * occurs.
 * @param methodCallback This is a function pointer that is to be used on
 * the callback.
 */
void GPIO::setCallbackMethod(CallbackType methodCallback) {
	// To begin, if the method callback is currently NULL, we will need to
	// start the thread.
	// Otherwise, if it is not NULL and we are NULLing it.
	// If it is not NULL and we are simply changing the method, then we do
	// not need to worry about anything other than changing the method.
	pthread_t thread;
	if (this->methodCallback == NULL) {
		this->methodCallback = methodCallback;

		int policy;
		struct sched_param sparam;

		// TODO This code is intended to start the new thread that will make
		// the callback at the same priority as the calling thread.
		// However, that is not working right now on the Pi, so this needs
		// Some improvement.

		pthread_attr_t tattr;
		pthread_attr_init(&tattr);
		pthread_getschedparam(pthread_self(), &policy, &sparam);

		pthread_create(&thread, &tattr, &startCallbackHander,
				static_cast<void*>(this));
		pthread_setschedparam(thread, policy, &sparam);
	} else {
		// This will handle the case of setting it to a new method and / or stopping the callback.
		this->methodCallback = methodCallback;
	}
}

/**
 * This method is used by the friend method to handle callback methods.
 * It should never be invoked from outside of the class.  In essence,
 * what it will do a loop so long as the callback method is not NULL,
 * invoking the callback method whenever an edge transitions.
 */
void GPIO::callBackThreadHandler() {
	while (this->methodCallback != NULL) {

		std::unique_lock<std::mutex> lck(mtx);
		ccv.wait(lck);
		this->methodCallback(0);
	}
}

/**
 * This method will obtain the GPIO Pin number associated with this instance of the GPIO class.
 * @return The unique pin number identifying the pin will be returned.
 */
int GPIO::getPinNumber() {
	return this->number;
}


} /* namespace exploringRPi */
