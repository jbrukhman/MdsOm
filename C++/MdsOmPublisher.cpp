
#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

//
MdsOmPublisherCallback::MdsOmPublisherCallback()
{
}

MdsOmPublisherCallback::~MdsOmPublisherCallback()
{
}

//
MdsOmPublisherFeedback::MdsOmPublisherFeedback()
{
	wthread_mutex_init(&feedbackLock, NULL);
}

MdsOmPublisherFeedback::~MdsOmPublisherFeedback()
{
	wthread_mutex_lock(&feedbackLock);		// make sure the lock is not in use before destroying it
	wthread_mutex_unlock(&feedbackLock);
	wthread_mutex_destroy(&feedbackLock);
}

#ifndef OM_241

// SOLACE feedback subscriber
void MdsOmPublisherFeedback::addPublisher(MdsOmPublisher* p, const char* topic, MamaTransport* transport, bool isSolace)
{
	MdsOmEnv* env = p->getEnv();

	// Check the config to turn on/off the Solace async publisher feedback
	char buf[255];
	snprintf(buf, sizeof(buf), "mama.%s.transport.%s.publisherFeedback", env->getBridgeName(), transport->getName());
	const char* feedStr = Mama::getProperty(buf);
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmPublisherFeedback::addPublisher: %s %s", buf, feedStr);
	if (NULL == feedStr || strcmp(feedStr, "true")) return;

	wthread_mutex_lock(&feedbackLock);
	try {
		if (isSolace) {
			set<MamaTransport*>::iterator it = transportMap.find(transport);
			if (it == transportMap.end()) {
				// Don't have existing feedback subscription for this transport, so add one
				// There is only 1 feedback sub per transport
				transportMap.insert(transport);

				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmPublisherFeedback::addPublisher: %s", transport->getName());
				MamaBasicSubscription* feedback = new MamaBasicSubscription();
				feedback->createBasic(transport, env->getQueue(), this, "_SOLACE.TOPIC_TRANSPORT_CB_EX", NULL);
			}
		}

		// Add the publisher to the list of listeners
		PublisherList* pl = feedbackMap[topic];
		if (pl == NULL) {
			pl = new PublisherList();
			feedbackMap[topic] = pl;
		}
		pl->insert(p);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisherFeedback::addPublisher: error with feedback %s %s", p->getSubject(), status.toString());
	}
	wthread_mutex_unlock(&feedbackLock);
}

// SOLACE feedback subscriber
void MdsOmPublisherFeedback::onCreate (MamaBasicSubscription*  subscription)
{
}

void MdsOmPublisherFeedback::onError (
    MamaBasicSubscription*  subscription,
    const MamaStatus&       status,
    const char*             topic)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmPublisherFeedback::onError: %s %s", topic, status.toString());
}

void MdsOmPublisherFeedback::onMsg (
    MamaBasicSubscription*  subscription,
    MamaMsg&                msg)
{
	// Received publish msg from Solace
	solacePublishFail(msg);
}

void MdsOmPublisherFeedback::onDestroy (
    MamaBasicSubscription* subscription,
    void*                  closure)
{
	const char* topic = subscription->getTopic();
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmPublisherFeedback::onDestroy: %s", topic);
}

// SOLACE & SUB FEEDBACK
// This is for temporary Solace publish feedback.
// Will be replaced by OM solution.
void MdsOmPublisherFeedback::solacePublishFail(MamaMsg& msg)
{
	// TODO lock is held for along time, but this is only for publish errors
	wthread_mutex_lock(&feedbackLock);
	try {
		// These are the fields that Solace sends
		mama_i32_t eventType = 0;
		mama_i32_t mamaStatus = 0;
		const char* statusText = "";
		const char* topic = "";
		bool found;
		
		// TODO BAD hardcoded, this is from the Solace bridge manual, but will be removed when OM publish cb is available
		found = msg.tryString(NULL, 12, topic);
		if (topic == NULL) {
			wthread_mutex_unlock(&feedbackLock);
			return;
		}

		found = msg.tryI32(NULL, 9, eventType);
		found = msg.tryString(NULL, 13, statusText);
		found = msg.tryI32(NULL, 14, mamaStatus);

		processPublishers(topic, (mama_status) mamaStatus, statusText);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::solacePublishFail: error %s",  status.toString());
	}
	wthread_mutex_unlock(&feedbackLock);
}
#endif

#ifdef OM_241
void MdsOmPublisherFeedback::addPublisher(MdsOmPublisher* p, const char* topic, MamaTransport* transport)
{
	MdsOmEnv* env = p->getEnv();

	wthread_mutex_lock(&feedbackLock);
	try {
		// Add the publisher to the list of listeners
		PublisherList* pl = feedbackMap[topic];
		if (pl == NULL) {
			pl = new PublisherList();
			feedbackMap[topic] = pl;
		}
		pl->insert(p);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisherFeedback::addPublisher: error with feedback %s %s", p->getSubject(), status.toString());
	}
	wthread_mutex_unlock(&feedbackLock);
}
#endif

void MdsOmPublisherFeedback::removePublisher(MdsOmPublisher* p)
{
	wthread_mutex_lock(&feedbackLock);
	try {
		PublisherList* pl = feedbackMap[p->getSubject()];
		if (pl) {
			pl->erase(p);
			if (pl->size() == 0) {
				// No publisher for this subject, remove it altogether
				feedbackMap.erase(p->getSubject());
				delete pl;
			}
		}
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisherFeedback::solacePublishFail: error with solace feedback %s %s", p->getSubject(), status.toString());
	}
	wthread_mutex_unlock(&feedbackLock);
}

void MdsOmPublisherFeedback::processTransportTopicPublishers(const char* subject, mama_status mamaStatus)
{
	// TODO lock is held for along time, but this is only for publish errors
	wthread_mutex_lock(&feedbackLock);
	try {
		processPublishers(subject, mamaStatus, (const char*) "");
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::processTransportTopicPublishers: error %s %s", subject, status.toString());
	}
	wthread_mutex_unlock(&feedbackLock);
}

void MdsOmPublisherFeedback::processPublishers(const char* subject, mama_status mamaStatus, const char* msg)
{
	try {
		PublisherList* pl = NULL;
		try {
			pl = feedbackMap[subject];
		} catch (MamaStatus& status) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisherFeedback::processPublishers: error with feedback map %s %s", subject, status.toString());
		}

		if (pl == NULL) {
			return;
		}

		MamaStatus mStatus((mama_status) mamaStatus);

		// Send feedback to each publisher of this symbol
		PublisherList::iterator it = pl->begin();
		while (it != pl->end()) {
			MdsOmPublisher* pub = *it++;

			try {
           		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmPublisher::processPublishers: %s mamaStatus=%d msg='%s'",
               		pub->getSubject(), mamaStatus, msg);

				MdsOmPublisherCallback* cb = pub->getCb();
				if (cb && mamaStatus != MAMA_STATUS_OK) {
					cb->onPublishError(pub, mStatus, pub->getClosure());
				}
			} catch (MamaStatus& status) {
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::processPublishers: error on cb %s status=%s msg=%s",
					subject, status.toString(), msg);
			}
		}
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::processPublishers: error %s status=%s msg=%s",
			subject, status.toString(), msg);
	}
}

// Static data 
MdsOmPublisherFeedback MdsOmPublisher::feedback;

//
MdsOmPublisher::MdsOmPublisher()
{
	cb = NULL;
	transport = NULL;
	closure = NULL;
	env = NULL;
	msg = NULL;
	valid = false;
#ifndef OM_241
	inbox = NULL;
#endif
}

MdsOmPublisher::~MdsOmPublisher()
{
}

void MdsOmPublisher::create(MdsOmEnv* env, MamaTransport* transport, const char* source, const char* symbol, MdsOmPublisherCallback* cb, void* closure)
{
	try {
		this->env = env;
		this->transport = transport;
#ifndef OM_241
		this->source = source;
		this->symbol = symbol;
#endif
		this->subject = string(source) + "." + string(symbol);
		this->cb = cb;
		this->closure = closure;

		if (!env->isConnected()) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::create: %s not connected", env->getName());
			throw MdsOmStatus(MDS_OM_STATUS_BRIDGE_DID_NOT_CONNECT);
		}

#ifdef OM_241
		feedback.addPublisher(this, subject.c_str(), transport);
		MamaPublisher::createWithCallbacks(transport, env->getQueue(), this, NULL, symbol, source);
#else
		if (env->getType() == MDS_OM_ENV_TREP) {
			// Tick42 - get feedback from inbox
			inbox = new MamaInbox();
			inbox->create(transport, env->getQueue(), this, NULL);
			MamaPublisher::create(transport, symbol, source);
		} else if (env->getType() == MDS_OM_ENV_MERCURY) {
			// Solace - get feedback from special subscription
			feedback.addPublisher(this, subject.c_str(), transport, true);
			MamaPublisher::create(transport, symbol, source);
		}
		if (cb) {
			cb->onPublishCreate(this, closure);
		}
#endif
	} catch (MamaStatus& status) {
		// Don't use the MamaPublisher here as it is not created
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmPublisher::create: error %s %s.%s %s", env->getName(), source, symbol, status.toString());
		feedback.removePublisher(this);
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOmPublisher::send(MamaMsg* msg) const
{
	try {
		env->countPubMsgs++;
		env->countPubMsgsPeriod++;

#ifdef OM_241
		MamaPublisher::send(msg);
#else
		if (env->getType() == MDS_OM_ENV_TREP && inbox != NULL) {
			// RMDS
			MamaPublisher::sendFromInbox(inbox, msg);
		} else {
			// Solace and Reflect
			MamaPublisher::send(msg);
		}
#endif
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmPublisher::send: error %s %s.%s %s", env->getName(), getSource(), getSymbol(), status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOmPublisher::destroy(void)
{
	try {
#ifdef OM_241
		feedback.removePublisher(this);
#else
		if (env->getType() == MDS_OM_ENV_MERCURY ||
			env->getType() == MDS_OM_ENV_REFLECT) {
			feedback.removePublisher(this);
		}
#endif

		env->countPubs--;

		// Do this before inbox to stop any callbacks to the inbox
		MamaPublisher::destroy();		// must be last as obj is deleted by onDestroy cb

#ifndef OM_241
		if (inbox != NULL) {
			inbox->destroy();
			delete inbox;
			inbox = NULL;
		}
		delete this;					// TODO this will change with publisher events
#endif

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::destroy: error %s %s", env->getName(), status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MamaMsg* MdsOmPublisher::getNewMamaMsg() const
{
	return newMamaMsg(env->getPayloadId());
}

void MdsOmPublisher::destroyMamaMsg(MamaMsg* msg)
{
	if (!msg) return;
	deleteMamaMsg(msg);
}

void MdsOmPublisher::storeMamaMsg(Wombat::MamaMsg* msg) { this->msg = msg; }
Wombat::MamaMsg* MdsOmPublisher::retrieveMamaMsg() const { return msg; }

MdsOmPublisherCallback* MdsOmPublisher::getCb() const { return cb; }

void* MdsOmPublisher::getClosure() const { return closure; }
void MdsOmPublisher::setClosure(void* closure) { this->closure = closure; }

bool MdsOmPublisher::isValid() const { return valid; }
void MdsOmPublisher::setValid(bool valid) { this->valid = valid; }

Wombat::MamaTransport* MdsOmPublisher::getMamaTransport() const { return transport; }

MamaDictionary* MdsOmPublisher::getDictionary() const
{
	return env->getDictionary();
}

const char* MdsOmPublisher::getSubject() const
{
	return subject.c_str();
}

MdsOmEnv* MdsOmPublisher::getEnv() const
{
	return env;
}

#ifndef OM_241
// This is used for Tick42, which returns publish status via inbox.
// Solace does it differently.
// OM group is working on a callback mechanism for publish.
void MdsOmPublisher::onDestroy(MamaInbox *inbox, 
                               void *closure)
{
}

void MdsOmPublisher::onMsg(MamaInbox* inbox,
                           MamaMsg& msg)
{
    mamaMsgType type = msg.getType();
    if (type == MAMA_MSG_TYPE_SEC_STATUS) {
		mama_i32_t result;
		bool found = msg.tryI32(MamaFieldMsgStatus.mName, MamaFieldMsgStatus.mFid, result);
		if (found == false) {
			result = MAMA_MSG_STATUS_UNKNOWN;
		}
        mamaMsgStatus status = (mamaMsgStatus) result;
		mama_status st = convertMsgStatusToMamaStatus(status);
		MamaStatus mStatus(st);
        if (mStatus != MAMA_STATUS_OK) {
            mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::onMsg: %s %s.%s type=%s status=%s",
                getEnv()->getName(), getSource(), getSymbol(),
                mamaMsgType_stringForType(type), mamaMsgStatus_stringForStatus(status));
			if (cb) {
				cb->onPublishError(this, mStatus, closure);
			}
        }
    }
}

void MdsOmPublisher::onError(MamaInbox* inbox,
                             const MamaStatus& status)
{
    mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmPublisher::onError: %s.%s status=%s",
        getSource(), getSymbol(), status.toString());
}

const char* MdsOmPublisher::getSymbol() const
{
	return symbol.c_str();
}

const char* MdsOmPublisher::getSource() const
{
	return source.c_str();
}

#else

// OpenMAMA feedback
void MdsOmPublisher::onCreate (
	MamaPublisher* publisher,
	void* closure)
{
	if (cb) {
		cb->onPublishCreate(this, this->closure);
	}
}

void MdsOmPublisher::onDestroy (
	MamaPublisher* publisher,
	void* closure)
{
	if (cb) {
		cb->onPublishDestroy(this, this->closure);
	}

	// All of the layers below have called onDestroy() and cleaned up
	delete this;
}

void MdsOmPublisher::onError (
	MamaPublisher* publisher,
	const MamaStatus& status,
	const char* info,
	void* closure)
{
	if (cb) {
		cb->onPublishError(this, status, this->closure);
	}
}

#endif

}

