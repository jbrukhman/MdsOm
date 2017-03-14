
/** 
 * @file MdsOmTransportTopicCallback.h
 * @brief Transport callback.
 * @see <twiki>
 */

#ifndef MdsOmTransportTopicCallback_H
#define MdsOmTransportTopicCallback_H

#include "MdsOm.h"

namespace MdsOmNs {

/**
 * This class provides OpenMama Transport Topic callbacks.
 *
 * It is optional, and can be passed in the MdsOmConfig class when creating an MdsOmEnv.
 * The difference to OpenMama callbacks is the addition of the MdsOmEnv* when getting a callback.
 */
class MDSOMExpDLL MdsOmTransportTopicCallback {
public:
	virtual ~MdsOmTransportTopicCallback() {}

	virtual void onTopicSubscribe (MdsOmEnv* env,
		                   Wombat::MamaTransport* tport,
                           const char* topic,
                           const void* platformInfo)
	{}

	virtual void onTopicUnsubscribe (MdsOmEnv* env,
		                     Wombat::MamaTransport* tport,
                             const char* topic,
                             const void* platformInfo)
	{}

	virtual void onTopicPublishError (MdsOmEnv* env,
		                      Wombat::MamaTransport* tport,
                              const char* topic,
                              const void* platformInfo)
	{}

	virtual void onTopicPublishErrorNotEntitled (MdsOmEnv* env,
		                                 Wombat::MamaTransport* tport,
                                         const char* topic,
                                         const void* platformInfo)
	{}

	virtual void onTopicPublishErrorBadSymbol (MdsOmEnv* env,
		                               Wombat::MamaTransport* tport,
                                       const char* topic,
                                       const void* platformInfo)
	{}
};

}

#endif
