
/** 
 * @file MdsOmTransportCallback.h
 * @brief Transport callback.
 * @see <twiki>
 */

#ifndef MdsOmTransportCallback_H
#define MdsOmTransportCallback_H

#include "MdsOm.h"

namespace MdsOmNs {

/**
 * This class provides OpenMama Transport callbacks.
 *
 * It is optional, and can be passed in the MdsOmConfig class when createing an MdsOmEnv.
 * The difference to OpenMama callbacks is the addition of the MdsOmEnv* when getting a callback.
 */
class MDSOMExpDLL MdsOmTransportCallback {
public:
	virtual ~MdsOmTransportCallback() {}

    /**
      * Invoked when the transport connects 
      *
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has connected.
      * @param platformInfo Info associated with the event.
      *
      * The cause and platformInfo are supplied only by some middlewares.
      * The information provided by platformInfo is middleware specific.
      * The following middlewares are supported:
      *
      * tibrv: provides the char* version of the tibrv advisory message.
      * wmw:   provides a pointer to a C mamaConnection struct for the event
      */
	virtual void onDisconnect(
		MdsOmEnv* env,
		Wombat::MamaTransport* transport,
		const void* platformInfo) = 0;

    /**
      * Invoked when the transport reconnects 
      *
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has reconnected.
      * @param platformInfo Info associated with the event.
      *
      * The cause and platformInfo are supplied only by some middlewares.
      * The information provided by platformInfo is middleware specific.
      * The following middlewares are supported:
      *
      * tibrv: provides the char* version of the tibrv advisory message.
      * wmw:   provides a pointer to a C mamaConnection struct for the event
      */
	virtual void onReconnect(
		MdsOmEnv* env,
		Wombat::MamaTransport* transport,
		const void* platformInfo) = 0;

    /**
      * Invoked when the quality of this transport changes.
      *
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport on which the quality has changed.
      * @param cause The cause of the quality event.
      * @param platformInfo Info associated with the quality event.
      *
      * The cause and platformInfo are supplied only by some middlewares.
      * The information provided by platformInfo is middleware specific.
      * The following middlewares are supported:
      *
      * tibrv: provides the char* version of the tibrv advisory message.
      */
    virtual void onQuality(
		MdsOmEnv* env,
        Wombat::MamaTransport*     transport,
        short              cause,
        const void*        platformInfo) = 0;

    /** 
      * Invoked on the subscriber when the transport connects.
      * 
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has connected.
      * @param platformInfo Info associated with the event.
      *
      * The cause and platformInfo are supplied only by some middlewares.
      * The information provided by platformInfo is middleware specific.
      * The following middlewares are supported:
      *
      * wmw:   provides a pointer to a C mamaConnection struct for the event
      */
    virtual void onConnect(
		MdsOmEnv* env,
        Wombat::MamaTransport*  transport,
        const void*     platformInfo) = 0;

    /**
      * Invoked on the publisher when the transport accepts a connection.
      *
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has accepted.
      * @param platformInfo Info associated with the event.
      *
      * The cause and platformInfo are supplied only by some middlewares.
      * The information provided by platformInfo is middleware specific.
      * The following middlewares are supported:
      *
      * wmw:   provides a pointer to a C mamaConnection struct for the event
      */
    virtual void onAccept(
		MdsOmEnv* env,
        Wombat::MamaTransport*   transport,
        const void*      platformInfo)
    {
        return;
    }

	/**
      * Invoked on the publisher when the transport accepts a reconnection.
      *
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has reconnected on
      * @param platformInfo Info associated with the event.
      *
      * The cause and platformInfo are supplied only by some middlewares.
      * The information provided by platformInfo is middleware specific.
      * The following middlewares are supported:
      *
      * wmw:   provides a pointer to a C mamaConnection struct for the event
      */
    virtual void onAcceptReconnect (
		MdsOmEnv* env,
        Wombat::MamaTransport*  transport,
        const void*     platformInfo)
    {
        return;
    }

    /**
      * Invoked on the subscriber when the transport disconnects from the publisher.
      *
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has disconnected on
      * @param platformInfo Info associated with the event.
      *
      * The cause and platformInfo are supplied only by some middlewares.
      * The information provided by platformInfo is middleware specific.
      * The following middlewares are supported:
      *
      * wmw:   provides a pointer to a C mamaConnection struct for the event
      */
    virtual void onPublisherDisconnect (
		MdsOmEnv* env,
        Wombat::MamaTransport*  transport,
        const void*     platformInfo)
    {
        return;
    }

    /** 
      * Invoked on the subscriber when the naming service connects.
      * 
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has connected.
      * @param platformInfo Info associated with the event.
      */
    virtual void onNamingServiceConnect (
		MdsOmEnv* env,
        Wombat::MamaTransport*  transport,
        const void*     platformInfo)
    {
        return;
    }

    /** 
      * Invoked on the subscriber when the naming service disconnects.
      * 
	  * @param env The MdsOmEnv that holds the transport.
      * @param transport The transport which has connected.
      * @param platformInfo Info associated with the event.
      */
    virtual void onNamingServiceDisconnect (
		MdsOmEnv* env,
        Wombat::MamaTransport*  transport,
        const void*     platformInfo)
    {
        return;
    }
};

}

#endif
