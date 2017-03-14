
#ifndef MdsOmLock_H
#define MdsOmLock_H

#include "MdsOm.h"

namespace MdsOmNs {

class MDSOMExpDLL MdsOmLock
{
private:
	wthread_mutex_t* lock;

public:
	MdsOmLock(wthread_mutex_t* lock);

	virtual ~MdsOmLock();
};

}

#endif
