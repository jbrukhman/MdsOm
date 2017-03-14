/** 
 * @file MdsOm.h
 * @brief Single include file for MdsOm.
 * @see <twiki>
 */

#ifndef MdsOm_H
#define MdsOm_H

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <cstddef>
#include <string>
#include <list>

#include <mama/mama.h>
#include <mama/mamacpp.h>
#include <mama/MamaReservedFields.h>

#ifdef WIN32
	#define MDSOMExpDLL __declspec(dllexport)
#else
	#include <errno.h>
	#define MDSOMExpDLL
#endif

namespace MdsOmNs {

// ===================================================================
class MdsOm;
class MdsOmEnv;
class MdsOmTransportCallback;
class MdsOmTransportTopicCallback;
class MdsOmPublisher;
class MdsOmPublisherCallback;
class MdsOmSubscription;
class MdsOmConfig;
class MdsOmStatus;
class MdsOmTransport;
class MdsOmTransports;
class MdsOmTransportSet;
class MdsOmLock;
class MdsOmDataLink;

// ===================================================================
/**
 * Enum for environment types.
 */
typedef enum _MdsOmEnvType {
	MDS_OM_ENV_MERCURY,				/**< Mercury/IDP/Solace. */ 
	MDS_OM_ENV_TREP,				/**< RMDS/TREP/Tick42. */ 
	MDS_OM_ENV_MAMACACHE,			/**< Firefly/MamaCache/SRLabs/Wombat. */ 
	MDS_OM_ENV_REFLECT,				/**< Reflect bridge */
	MDS_OM_ENV_UNKNOWN
} MdsOmEnvType;

// ===================================================================
typedef vector<string> MdsOmSourceCollection;
typedef map<string, MdsOmSourceCollection> MdsOmSourcesMap;

typedef vector<MdsOmTransport*> MdsOmTransportCollection;
typedef map<string, MdsOmTransportSet*> MdsOmTransportMap;

typedef vector<MdsOmEnv*> MdsOmEnvsCollection;
typedef vector<string> MdsOmEnvCollection;

typedef map<MdsOmSubscription*, MdsOmSubscription*> MdsOmSubMap;

typedef map<string, MdsOmPublisher*> MdsOmSolacePubMap;
}

// ===================================================================
#include "MdsOmLock.h"
#include "MdsOmList.h"
#include "MdsOmTransportCallback.h"
#include "MdsOmTransportTopicCallback.h"
#include "MdsOmSubscription.h"
#include "MdsOmConfig.h"
#include "MdsOmPublisher.h"
#include "MdsOmStatus.h"
#include "MdsOmTransports.h"
#include "MdsOmEnv.h"

namespace MdsOmNs {

// ===================================================================
/**
 * Main class for MdsOm.
 *
 * There is one of these for each application using market data. It manages each of the environments (e.g., RMDS/TREP, Wombat/Firefly, Mercury/Solace).
 */
class MDSOMExpDLL MdsOm : public Wombat::MamaTimerCallback, public Wombat::MamaStartCallback {
public:

	/**
	 * Constructor.
	 */
	MdsOm();

	/**
	 * Constructor.
	 * @param logFile	The logfile name to use. All MdsOm and OpenMAMA output is written to this.
	 * @param level		MamaLogLevel - the log level
	 */
	MdsOm(const char* logFile, MamaLogLevel level = MAMA_LOG_LEVEL_NORMAL);

	virtual ~MdsOm();

	/**
	 * Add an environment to the system.
	 * @param config  The config for the env.
	 * @return        Pointer to the new env.
	 * @throws        MamaStatus, MdsOmStatus
	 */
	MdsOmEnv* addEnv(MdsOmConfig& config);

	/**
	 * Return an env.
	 * @param type  The type of the env.
	 * @return      Pointer to the env.
	 * @throws      MamaStatus, MdsOmStatus
	 */
	MdsOmEnv* getEnv(MdsOmEnvType type) const;

	/**
	 * Get an env that will be used for a source or topic.
	 * This can be used to transfer control to the thread that will control this topic to keep all work in one thread.
	 * @param	topic - this can be either a source (e.g., BBG_BPIPE_DEV or PB) or a topic (e.g., BBG_BPIPE_DEV.TWTR.US or PB.APP.lonres).
	 * @return	env - the env that the symbol will be routed to. This routing is determined in the mama.properties file in the sourceMap property.
	 */
	MdsOmEnv* getEnvFromSymbol(const char* topic) const;

	/**
	 * Subscribe to a symbol.
	 * @param topic    The topic, this has both a source and symbol in it, like IDN_DEV.TWTR.N
	 * @param cb       The interface to send callbacks to.
	 * @param closure  The closure to send with each callback, can be NULL.
	 * @param setup    If true then call MamaSubscription setup instead of create, this requires later calling activate() to make the sub startup.
	 * @return         The subscription object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	MdsOmSubscription* subscribe(const char* topic, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup = false);

	/**
	 * Subscribe to a symbol.
	 * @param source   The source, e.g., IDN_DEV or PB or cBPOD_DEV
	 * @param symbol   The symbol, this is just the symbol, e.g., like TWTR.N
	 * @param cb       The interface to send callbacks to.
	 * @param closure  The closure to send with each callback, can be NULL.
	 * @param setup    If true then call MamaSubscription setup instead of create, this requires later calling activate() to make the sub startup.
	 * @return         The subscription object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	MdsOmSubscription* subscribe(const char* source, const char* symbol, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup = false);

	/**
	 * Snapshot a symbol, only the Initial is returned. No unsubscribe is required.
	 * @param topic    The topic, this has both a source and symbol in it, like IDN_DEV.TWTR.N
	 * @param cb       The interface to send callbacks to.
	 * @param closure  The closure to send with each callback, can be NULL.
	 * @param setup    If true then call MamaSubscription setup instead of create, this requires later calling activate() to make the sub startup.
	 * @return         The subscription object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	MdsOmSubscription* snap(const char* topic, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup = false);

	/**
	 * Snapshot a symbol, only the Initial is returned. No unsubscribe is required.
	 * @param source   The source, e.g., IDN_DEV or PB or cBPOD_DEV
	 * @param symbol   The symbol, this is just the symbol, e.g., like TWTR.N
	 * @param cb       The interface to send callbacks to.
	 * @param closure  The closure to send with each callback, can be NULL.
	 * @param setup    If true then call MamaSubscription setup instead of create, this requires later calling activate() to make the sub startup.
	 * @return         The subscription object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	MdsOmSubscription* snap(const char* source, const char* symbol, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup = false);

	/**
	 * Create a publisher for a symbol.
	 * @param topic    The topic, this has both a source and symbol in it, like IDN_DEV.TWTR.N
	 * @return         The publisher object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	MdsOmPublisher* getPublisher(const char* topic, MdsOmPublisherCallback* cb, void* closure);

	/**
	 * Create a publisher for a symbol.
	 * @param source   The source, e.g., IDN_DEV or PB or cBPOD_DEV
	 * @param symbol   The symbol, this is just the symbol, e.g., like TWTR.N
	 * @return         The publisher object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	MdsOmPublisher* getPublisher(const char* source, const char* symbol, MdsOmPublisherCallback* cb, void* closure);

	/**
	 * Open the MdsOm and OpenMama. This is called after the MdsOmEnv are setup. It will load the OpenMama bridges and connect to the infrastucture.
	 * @path path      the path to properties file, NULL means look in WOMBAT_PATH.
	 * @filename       the filename for properties file. NULL means mama.properties.
	 * @return         None.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void open(const char* path = NULL, const char* filename = NULL);

	/**
	 * Start the MdsOm and OpenMama. This <b>blocks</b>. It will begin the OpenMama processing of the queues. Any subscriptions created before this are now processed.
	 * @return         None.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void start(bool allBg = false);

	/**
	 * Close the MdsOm and OpenMama. This will shut down all of the infrastructure connections and stop processing the queues.
	 * @return         None.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void close();

	/**
	 * Get a queue for processing timer.
	 * @return         The MamaQueue.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	Wombat::MamaQueue* getTimerQueue() const;

	int getStatsInterval() const;

	/**
	 * Set the statistics interval, default is 30 secs.
	 * Set to 0 to turn off the statics collection and output.
	 * @param statsInterval    The interval in seconds.
	 * @return         None.
	 */
	void setStatsInterval(int statsInterval);

	/**
	 * Timer callback
	 */
    void onTimer(Wombat::MamaTimer* timer);

	/**
	 * Timer destroy callback
	 */
    void onDestroy(Wombat::MamaTimer * timer, void * closure);

	/**
	  * Called by OpenMama when each background bridge has finished initializing.	
	  */
	void onStartComplete (Wombat::MamaStatus status);

	/**
	  * Set the number of waits when connecting to the infrastructure.
	  * When waiting for the infra MdsOm sleeps for a number of times (waits).
	  */
	void setWaits(int waits);

	/**
	  * Get the waits
	  */
	int getWaits() const;

	/**
	 * Set whether to return an error if all connections fail, or to wait for full timeout.
	 */
	void setFullTimeout(bool fullTimeout);
	bool getFullTimeout() const;

	/**
	 * Get the log file name
	 */
	const char* getLogFile() const;

	/**
	 * Set to allow multiple envs to same infra.
	 * Normally we enforce one Trep, one Mercury, etc.
	 * With this called we allow multiple of each, but the app must manage the subscriptions via MdsOmEnv, since the 
	 *  subscribe calls in MdsOm use a source router, and may route a sub to a different MdsOmEnv than the app intended.
	 */
	void setMultipleEnvs();

private:
	MdsOmEnv* findEnv(MdsOmEnvsCollection envs, const char* source) const;
	void cleanup();
	void stopQueues();
	void init(const char* logFile, MamaLogLevel level);

	// ENVIRONMENTS
	MdsOmEnv* idp;
	MdsOmEnv* rmds;
	MdsOmEnv* firefly;
	MdsOmEnv* reflect;
	MdsOmEnvsCollection envs;
	bool multipleEnvs;

	// For logging
	const char* logFile;						// log file name, default is NULL, which is stderr
	FILE* logFileFp;

	int statsInterval;							// statistics interval, if 0 then disable stats
	wsem_t doneSem;								// used to synchronize the shutdown

	Wombat::MamaTimer statsTimer;

	bool allBg;									// true = all envs are started in background
	bool didStart;								// true = called start, so we can call stop
	int waits;									// used to wait for transport/dictionaries/etc
	bool fullTimeout;							// true = wait for full timeout before returning an connection error to the app
};

}

/*! \mainpage Market Data Services OpenMAMA Helper Classes
 *
 * \section intro_sec Introduction
 *
 * MdsOm and its associated classes provide a streamlined way to use OpenMAMA for market data.
 *
 * The best way to get familiar is to look at the example programs in this install.
 * All of the sample programs use a common class MdsOmSampleCommon located in the common dir.
 *
 * There are four sample programs:

 * 1. MdsOmSampleSub - This shows how to subscribe, using either symbols on the command line or a file of symbols.
 *
 * 2. MdsOmSamplePub - This shows how to publish, using either symbols on the command line or a file of symbols.
 *
 * 3. MdsOmSample - This shows how to publish and subscribe.
 *
 * 4. omric - This is a simple subscription program like showric (MDSAPI) or metaric (MetaFluent).
 *
 */

#endif
