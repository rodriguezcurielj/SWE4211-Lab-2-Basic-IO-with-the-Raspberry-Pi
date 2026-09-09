/**
 * @file GPIO.h
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

#ifndef WiringPiGPIO_H_
#define WiringPiGPIO_H_

#include "GPIO.h"

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

#define NUMBER_OF_GPIO_PINS (28)

namespace SWE4211RPi {

typedef int (*CallbackType)(int);

/**
 * @class GPIO
 * @brief GPIO GPIO wrapper classes for usage with C++ and wiringPi.
 */
class WiringPiGPIO : public GPIO{
public:
	/**
	 * This is the constructor for an instance of a GPIO class.
	 * @param number This is the Raspberry Pi GPIO Number.
	 * @param dir This is the direction for the GPIO pin, either input or
	 * output.
	 * @param val This is the default value for the GPIO pin.
	 */
	WiringPiGPIO(int number, DIRECTION dir, VALUE val = GPIO_LOW);

	/**
	 * This is the destructor for the class, which will clean up the existing
	 * instance.
	 */
	virtual ~WiringPiGPIO();  //destructor will unexport the pin

protected:
	/**
	 * This is the GPIO pin number for the given pin.
	 */
	int number;

private:


	/**
	 * This attribute indicates whether any GPIO has been instantiated.
	 * The first GPIO to be instantiated will also setup the wiringPi devices.
	 * Subsequent instantiations will not do so.
	 */
	static int initialized;

public:
	// General Input and Output Settings
	/**
	 * This method will drive an output pin to the appropriate value.
	 * @param This is the value that is to be written to the pin.
	 */
	virtual void setValue(GPIO::VALUE);

	/**
	 * This method will return the current state of this GPIO pin.
	 * @return The return will be high or low depending on the state of the pin.
	 */
	virtual GPIO::VALUE getValue();

// Edge and Interrupt Methods

	/**
	 * This method will enable an interrupt to be processed based upon edges.
	 * In doing this, callbacks can be made based upon the GPIO pin changing
	 * state and methods can block for IO purposes.
	 * @param edgeType This is the edge type that is to be processed as an
	 * interrupt.
	 */
	virtual void enableEdgeInterrupt(EDGE edgeType);



	/**
	 * This method will be invoked if an interrupt is enabled whenever a
	 * transition in pin value occurs.
	 */
	virtual void handleInterruptDrivenGPIOPin();

};

} /* namespace SWE4211RPi */

#endif /* GPIO_H_ */
