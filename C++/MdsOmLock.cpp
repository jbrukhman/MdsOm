
#include "MdsOmLock.h"

namespace MdsOmNs {

MdsOmLock::MdsOmLock(wthread_mutex_t* lock)
{
	this->lock = lock;
	wthread_mutex_lock(lock);
}

MdsOmLock::~MdsOmLock()
{
	wthread_mutex_unlock(lock);
}

}

