
#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

MdsOmSubscription::MdsOmSubscription() : localCb(*this)
{
	gotResponse = false;
	cb = NULL;
	env = NULL;
	waitingForDestruct = false;
	deleteInOnDestroy = false;
	msgs = 0;
	wsem_init(&sem, 0, 0);
}

MdsOmSubscription::~MdsOmSubscription()
{
	// This must be called from the same thread that the sub gets its messages from via its queue

	// don't delete localCb here, see below
	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::~: %s source=%s symbol=%s", env->name, getSubscSource(), getSymbol());

	wsem_destroy(&sem);
}

void MdsOmSubscription::destroy()
{
	internalDestroy(false, 0, false);
}

int MdsOmSubscription::destroy(size_t timeout)
{
	return internalDestroy(true, timeout, false);
}

void MdsOmSubscription::destroyEx()
{
	internalDestroy(false, 0, true);
}

int MdsOmSubscription::destroyEx(size_t timeout)
{
	return internalDestroy(true, timeout, true);
}

int MdsOmSubscription::internalDestroy(bool useSem, size_t interval, bool useEx)
{
	int ret = 0;
	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::destroy: %s source=%s symbol=%s sem=%d int=%d ex=%d",
		env->name, getSubscSource(), getSymbol(), useSem, interval, useEx);
	try {
		waitingForDestruct = true;
		if (!useSem) deleteInOnDestroy = true;

		if (useEx) {
			MamaSubscription::destroyEx();
		} else {
			MamaSubscription::destroy();
		}

		if (useSem) {
			if (interval) {
				ret = wsem_timedwait(&sem, (unsigned int) interval);
			} else {
				ret = wsem_wait(&sem);
			}
			if (ret < 0) {
#ifdef WIN32
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::internalDestroy: semaphore wait failed %s ret=%d", getTopic(), ret);
#else
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::internalDestroy: semaphore wait failed %s ret=%d errno=%d", getTopic(), ret, errno);
#endif
			}

			// The onDestroy has occurred, so delete this now and return to the app
			delete this;
		}
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::destroy: error %s %s", env->name, status.toString());
		// Send original exception reference back to caller
		throw;
	}
	return ret;
}

MamaDictionary* MdsOmSubscription::getDictionary() const
{
	return env ?  env->getDictionary() : NULL;
}

void MdsOmSubscription::snap(MdsOmEnv* env, MamaTransport* transport, const char* source, const char* symbol, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	createInternal(env, transport, source, symbol, cb, closure, setup, MAMA_SERVICE_LEVEL_SNAPSHOT);
}

void MdsOmSubscription::create(MdsOmEnv* env, MamaTransport* transport, const char* source, const char* symbol, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	createInternal(env, transport, source, symbol, cb, closure, setup, MAMA_SERVICE_LEVEL_REAL_TIME);
}

void MdsOmSubscription::createInternal(MdsOmEnv* env, MamaTransport* transport, const char* source, const char* symbol, MamaSubscriptionCallback* cb, void* closure, bool setup, mamaServiceLevel subType)
{
	try {
		this->env = env;
		this->cb = cb;

		if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::createInternal: %s cb=%p source=%s symbol=%s setup=%d type=%d",
			env->name, cb, source, symbol, setup, subType);
		// would use mamaServiceLevel_toString() but it gives an odd link error

		if (!env->isConnected()) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::createInternal: %s not connected", env->getName());
			throw MdsOmStatus(MDS_OM_STATUS_BRIDGE_DID_NOT_CONNECT);
		}

		MamaQueue* queue = env->queueGroup->getNextQueue();
		if (subType == MAMA_SERVICE_LEVEL_SNAPSHOT) {
			setRequiresInitial(true);
		} else {
			setRequiresInitial(env->requireInitial);
		}
		setSubscriptionType(MAMA_SUBSC_TYPE_NORMAL);
		setServiceLevel(subType, 0);
		if (setup) {
			MamaSubscription::setup(transport,
							queue,
							&this->localCb,
							source,
							symbol,
							closure);
		} else {
			MamaSubscription::create(transport,
							queue,
							&this->localCb,
							source,
							symbol,
							closure);
		}
		setRecoverGaps(env->enableDq);			// done after create since it is set by create TODO race condition?
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::createInternal: error %s %s.%s %s", env->name, source, symbol, status.toString());
		throw status;
	}
}

// --- SUBSCRIPTION CB -----------------------------------------------------------------
void MdsOmSubscription::MdsOmSubscriptionCb::onMsg(MamaSubscription* sub, MamaMsg& msg)
{
	MdsOmSubscription* subscription = (MdsOmSubscription*) sub;
	if (subscription->waitingForDestruct) {
		return;
	}
	subscription->msgs++;

	try {
		mama_u64_t start = 0;
		if (parent.env->enableStats) {
			// TODO faster to just get the now?
			start = getNow();

			 const char* symbol = subscription->getTopic();
			 const char* source = subscription->getSubscSource();
			 mamaMsgType type = MAMA_MSG_TYPE_UNKNOWN;
			 mamaMsgStatus status = MAMA_MSG_STATUS_UNKNOWN;
			 try {
				 type = msg.getType();
				 status = msg.getStatus();
			} catch (MamaStatus& stat) {
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::onMsg: %s %s error getting type=%d and status=%d %s",
					parent.env->name, symbol, type, status, stat.toString());
			}

			if (parent.env->countTransport > 1) wthread_mutex_lock(&parent.env->statsLock);

			parent.env->countTypes(type, status);

			MamaQueue* q = subscription->getQueue();
			mama_size_t c = q->getEventCount();					// calc'd on a per-queue basis
			mama_u64_t end = *(mama_u64_t*) q->getClosure();

			if (c > parent.env->maxQueueCount) parent.env->maxQueueCount = c;

			// Get timing data
			if (end != 0) {
				parent.env->btwnOnMsgSum += start - end;
				parent.env->btwnOnMsgCount++;
			}

			// Count how many subs have received a response
			if (!subscription->gotResponse) {
				subscription->gotResponse = true;
				parent.env->countResponses++;
			}

			parent.env->countSubMsgs++;
			parent.env->countSubMsgsPeriod++;

			if (parent.env->countTransport > 1) wthread_mutex_unlock(&parent.env->statsLock);
		}

		if (subscription->cb) {
			subscription->cb->onMsg(subscription, msg);
		}

		if (parent.env->enableStats) {
			mama_u64_t end = getNow();
			if (parent.env->countTransport > 1) wthread_mutex_lock(&parent.env->statsLock);
			MamaQueue* q = subscription->getQueue();
			mama_u64_t* endp = (mama_u64_t*) q->getClosure();
			// TODO check for NULL
			*endp = end;
			parent.env->inOnMsgSum += end - start;
			parent.env->inOnMsgCount++;
			if (parent.env->countTransport > 1) wthread_mutex_unlock(&parent.env->statsLock);
		}

	} catch (MamaStatus& status) {
		const char* symbol = subscription->getTopic();
		const char* source = subscription->getSubscSource();
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::onMsg: error %s %s.%s %s",
			parent.env->name, source, symbol, status.toString());
	}
}

void MdsOmSubscription::MdsOmSubscriptionCb::onCreate(MamaSubscription* sub)
{
	MdsOmSubscription* subscription = (MdsOmSubscription*) sub;
	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::onCreate: %s %s.%s", parent.env->name, sub->getSubscSource(), sub->getSymbol());
	try {
		subscription->cb->onCreate(subscription);
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::onCreate: error %s %s %s", parent.env->name, subscription->getTopic(), status.toString());
	}
}

void MdsOmSubscription::MdsOmSubscriptionCb::onError(MamaSubscription* sub, const MamaStatus& status, const char* symbol)
{
	MdsOmSubscription* subscription = (MdsOmSubscription*) sub;
	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::onError: %s %s.%s %s",
		parent.env->name, sub->getSubscSource(), sub->getSymbol(), status.toString());
	try {
		if (parent.env->enableStats) {
			if (parent.env->countTransport > 1) wthread_mutex_lock(&parent.env->statsLock);

			if (!subscription->gotResponse) {
				subscription->gotResponse = true;
				parent.env->countResponses++;
			}
			mamaMsgStatus msgStatus =  convertMamaStatusToMsgStatus(status.getStatus());
			parent.env->countTypes(MAMA_MSG_TYPE_SEC_STATUS, msgStatus);

			if (parent.env->countTransport > 1) wthread_mutex_unlock(&parent.env->statsLock);
		}
		subscription->cb->onError(subscription, status, symbol);
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::onError: error %s %s %s", parent.env->name, symbol, status.toString());
	}
}

void MdsOmSubscription::MdsOmSubscriptionCb::onGap(MamaSubscription* sub)
{
	MdsOmSubscription* subscription = (MdsOmSubscription*) sub;
	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::onGap: %s %s.%s",
		parent.env->name, sub->getSubscSource(), sub->getSymbol());
	try {
		subscription->cb->onGap(subscription);
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::onGap: error %s %s %s", parent.env->name, parent.getSymbol(), status.toString());
	}
}

void MdsOmSubscription::MdsOmSubscriptionCb::onDestroy(MamaSubscription* sub)
{
	MdsOmSubscription* subscription = (MdsOmSubscription*) sub;
	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::onDestroy: %s destruct=%d %s.%s",
		parent.env->name, parent.waitingForDestruct, sub->getSubscSource(), sub->getSymbol());

	parent.env->countSubs--;

	subscription->cb->onDestroy(sub);

	if (subscription->deleteInOnDestroy) {
		// Not waiting on sem, delete now
		delete subscription;
	} else {
		// Post sem in that destroy() is waiting on
		wsem_post(&subscription->sem);
	}
}

void MdsOmSubscription::MdsOmSubscriptionCb::onRecapRequest(MamaSubscription* sub)
{
	MdsOmSubscription* subscription = (MdsOmSubscription*) sub;
	try {
		subscription->cb->onRecapRequest(subscription);
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::onRecapRequest: error %s %s %s", parent.env->name, parent.getSymbol(), status.toString());
	}
}

void MdsOmSubscription::MdsOmSubscriptionCb::onQuality(MamaSubscription* sub, mamaQuality quality, const char* symbol, short cause, const void* platformInfo)
{
	MdsOmSubscription* subscription = (MdsOmSubscription*) sub;
	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmSubscription::onQuality: %s %s.%s", parent.env->name, sub->getSubscSource(), sub->getSymbol());

	if (parent.env->enableStats) {
		if (parent.env->countTransport > 1) wthread_mutex_lock(&parent.env->statsLock);
		parent.env->countOnQuality(quality);
		if (parent.env->countTransport > 1) wthread_mutex_unlock(&parent.env->statsLock);
	}

	try {
		subscription->cb->onQuality(subscription, quality, symbol, cause, platformInfo);
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmSubscription::onQuality: error %s %s quality=%s cause=%d %s",
			parent.env->name, symbol, mamaQuality_convertToString(quality), cause, status.toString());
	}
}

}
