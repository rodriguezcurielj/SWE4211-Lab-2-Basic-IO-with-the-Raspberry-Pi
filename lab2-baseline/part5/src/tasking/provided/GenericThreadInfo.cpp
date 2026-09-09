#include "GenericThreadInfo.h"
#include <sys/resource.h>
#include <sched.h>            /* Definition of SCHED_* constants */
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>

using namespace SWE4211RPi;


/**
 * This is the default constructor for the class.
 * @param threadName This is the name of the thread in a human readable format.
 */
GenericThreadInfo::GenericThreadInfo(std::string threadName, pid_t myThreadID) :
		RunnableClass(threadName) {
	// Obtain the thread id by making a system call.
	myOSThreadID = myThreadID;
	int which = PRIO_PROCESS;
	this->priority = getpriority(which, myThreadID);

	struct sched_attr temp;

    syscall(SYS_sched_getattr, myThreadID, &temp, sizeof(struct sched_attr), 0);
    this->priority = temp.sched_priority;
}

/**
 * This method will clean up from the periodic task and its execution.
 */
GenericThreadInfo::~GenericThreadInfo() {
	// Nothing to do in the destructor.

}

/**
 * This is the run method for the class.
 */
void GenericThreadInfo::run() {
	// Do nothing, as this really is just for placeholder purposes.
}

