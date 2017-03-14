#ifndef MdsOmEnv_H
#define MdsOmEnv_H

#include "MdsOm.h"

namespace MdsOmNs {

/**
 * One of these per environment.
 *
 * There is one of these for each of the environments (e.g., RMDS/TREP, Wombat/Firefly, Mercury/Solace).
 */
class MDSOMExpDLL MdsOmEnv : public Wombat::MamaTransportCallback, public Wombat::MamaTransportTopicEventCallback, public Wombat::MamaDictionaryCallback {
public:
	friend class MdsOm;
	friend class MdsOmPublisher;
	friend class MdsOmSubscription;

	MdsOmEnv();

	/**
	 * Subscribe to a symbol for this env.
	 * @param topic        The topic (e.g., IDN_DEV.TWTR.N).
	 * @param cb           A Mama subscription callback to get data on.
	 * @param closure      Closure that is returned on each callback.
	 * @param setup        True to call setup() rather than create() on the sub. If this is true the activate() needs to be called on the sub to start getting data.
	 * @return             MdsOmSubscription instance.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	MdsOmSubscription* subscribe(const char* topic, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup = false);

	/**
	 * Subscribe to a symbol for this env.
	 * @param source       The source (e.g., IDN_DEV).
	 * @param symbol       The symbol (e.g., TWTR.N).
	 * @param cb           A Mama subscription callback to get data on.
	 * @param closure      Closure that is returned on each callback.
	 * @param setup        True to call setup() rather than create() on the sub. If this is true the activate() needs to be called on the sub to start getting data.
	 * @return             MdsOmSubscription instance.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	MdsOmSubscription* subscribe(const char* source, const char* symbol, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup = false);

	/**
	 * Snapshot a symbol, only the Initial is returned. No unsubscribe is required.
	 * @param topic        The topic (e.g., IDN_DEV.TWTR.N).
	 * @param cb           A Mama subscription callback to get data on.
	 * @param closure      Closure that is returned on each callback.
	 * @param setup        True to call setup() rather than create() on the sub. If this is true the activate() needs to be called on the sub to start getting data.
	 * @return             MdsOmSubscription instance.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	MdsOmSubscription* snap(const char* topic, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup = false);

	/**
	 * Snapshot a symbol, only the Initial is returned. No unsubscribe is required.
	 * @param source       The source (e.g., IDN_DEV).
	 * @param symbol       The symbol (e.g., TWTR.N).
	 * @param cb           A Mama subscription callback to get data on.
	 * @param closure      Closure that is returned on each callback.
	 * @param setup        True to call setup() rather than create() on the sub. If this is true the activate() needs to be called on the sub to start getting data.
	 * @return             MdsOmSubscription instance.
	 * @throws             MamaStatus, MdsOmStatus
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
	 * Get a MamaMsg to publish.
	 * This method handles setting the correct payload id for this publisher.
	 * The msg is owned by the caller, but can be reused for the life of the application.
	 * @return             MamaMsg*.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	Wombat::MamaMsg* getMamaMsg() const;

	/**
	 * Destroy a MamaMsg.
	 * @param              msg the MamaMsg to destroy.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	void destroyMamaMsg(Wombat::MamaMsg* msg);

	/**
	 * Return the enumerated type of the env.
	 * @return         MdsOmEnvType.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	MdsOmEnvType getType() const { return type; }

	/**
	 * Set the full path to a Mama dictionary file. This is rarely used as the dictionary is normally downloaded from the infrastructure automatically.
	 * @param fileName The full path.
	 * @return         None.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void setDictionaryFile(const char* fileName);

	/**
	 * Is this IDP/Mercury?
	 * @return             bool.
	 */
	bool isIdp() const { return type == MDS_OM_ENV_MERCURY; }

	/**
	 * Is this RMDS/TREP?
	 * @return             bool.
	 */
	bool isRmds() const { return type == MDS_OM_ENV_TREP; }

	/**
	 * Is this FireFly/MamaCache?
	 * @return             bool.
	 */
	bool isFirefly() const { return type == MDS_OM_ENV_MAMACACHE; }

	/**
	 * Can this env fail to start but still allow other envs to run?
	 * Use carefully if this is true.
	 * @return             bool.
	 */
	bool getCanThisEnvFailToStart() { return config.getCanThisEnvFailToStart(); }

	/**
	 * Has the dictionary loaded OK?
	 * @return             bool.
	 */
	bool isDictionaryOk() const { return dictionaryStatus == MDS_OM_ENV_STATUS_SUCCESS; }

	/**
	 * Is the dictionary waiting to be loaded?
	 * @return             bool.
	 */
	bool isDictionaryWait() const { return dictionaryStatus == MDS_OM_ENV_STATUS_WAIT; }

	/**
	 * Has the dictionary failed to load?
	 * @return             bool.
	 */
	bool isDictionaryFail() const { return dictionaryStatus == MDS_OM_ENV_STATUS_FAIL; }

	/**
	 * Does this env have OpenMama Data Quality enabled?
	 * @return             bool.
	 */
	bool isEnableDq() const { return enableDq; }

	/**
	 * Set OpenMama Data Quality enabled.
	 * @param              enableDq
	 * @return             None.
	 */
	void setEnableDq(bool enableDq) { this->enableDq = enableDq; }

	/**
	 * Get the Mama bridge object. This is normally not needed.
	 * @return         mamaBridge.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	mamaBridge getBridge() const { return bridge; }

	/**
	 * Get the Mama queue object. This is normally not needed, unless the app wants to use mama user events.
	 * @return         MamaQueue.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	Wombat::MamaQueue* getQueue() const { return queueGroup->getNextQueue(); }

	/**
	 * Get the Mama dictionary object. The dictionary is used to get information about Mama fields (e.g., fid, name,type).
	 * @return         MamaDictionary.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	Wombat::MamaDictionary* getDictionary() const { return dictionary; }

	/**
	  * Write the MamaDictionary for this env to the file.
	  * @param		fileName to write to.
	  * @returns	bool success or fail
	  */
	bool writeDictionary(const char* fileName);

	/**
	  * Get the middleware bridge name.
	  */
	const char* getBridgeName() const { return bridgeName; }

	/**
	  * Get the middleware transport name.
	  */
	const char* getTransportName() const { return config.transportName.c_str(); }

	/**
	  * Get the env name, used for logging
	  */
	const char* getName() const { return name; }

	/**
	  * Get the MamaMsg payload id for this middleware bridge.
	  */
	mamaPayloadType getPayloadId() const { return payloadId; }

	/**
	 * Get a field name given its fid.
	 * @param		   field fid.
	 * @return         field name.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	const char* getFieldName(mama_fid_t fid);

	/**
	 * Get a field fid given its name.
	 * @param		   field name.
	 * @return         field fid.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	mama_fid_t getFieldFid(const char* name);

	/**
	 * Get a field descriptor given its fid.
	 * @param		   field fid.
	 * @return         field descriptor.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	Wombat::MamaFieldDescriptor* getFieldDescriptor(mama_fid_t fid);

	/**
	 * Get a field descriptor given its name.
	 * @param		   field name.
	 * @return         field descriptor.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	Wombat::MamaFieldDescriptor* getFieldDescriptor(const char* name);

	/**
	 * Get a vector of the transports for this env.
	 * @param			reference to vector
	 */
	void getTransports(vector<Wombat::MamaTransport*>& transports);

	bool isConnected() const { return transports.isConnected(); }

	/**
	 * Is this env waiting for a connection?
	 * @return             bool.
	 */
	bool isConnectWait() const { return transports.isConnectWait(); }

	/**
	 * Is this env failed a connection?
	 * @return             bool.
	 */
	bool isConnectFail() const { return transports.isConnectFail(); }

	// ----------------------------------------------------------
	// MamaDictionaryCallback
    void onTimeout(void);
    void onError(const char* message);
    void onComplete(void);

	// ----------------------------------------------------------
	// MamaTransportCallback
	/** @see MdsOmTransportCallback */
    void onDisconnect(Wombat::MamaTransport* transport, const void* platformInfo);
	/** @see MdsOmTransportCallback */
    void onReconnect(Wombat::MamaTransport* transport,const void* platformInfo);
	/** @see MdsOmTransportCallback */
    void onQuality(Wombat::MamaTransport* transport, short cause, const void* platformInfo);
	/** @see MdsOmTransportCallback */
	void onConnect(Wombat::MamaTransport* transport, const void* platformInfo);
	/** @see MdsOmTransportCallback */
    void onAccept(Wombat::MamaTransport* transport, const void* platformInfo);
	/** @see MdsOmTransportCallback */
    void onAcceptReconnect(Wombat::MamaTransport* transport, const void* platformInfo);
	/** @see MdsOmTransportCallback */
    void onPublisherDisconnect(Wombat::MamaTransport* transport, const void* platformInfo);
	/** @see MdsOmTransportCallback */
    void onNamingServiceConnect(Wombat::MamaTransport* transport, const void* platformInfo);
	/** @see MdsOmTransportCallback */
    void onNamingServiceDisconnect(Wombat::MamaTransport* transport, const void* platformInfo);

	// ----------------------------------------------------------
	// MamaTransportTopicEventCallback
	void onTopicSubscribe (Wombat::MamaTransport* tport, const char* topic, const void* platformInfo);
	void onTopicUnsubscribe (Wombat::MamaTransport* tport, const char* topic, const void* platformInfo);
	void onTopicPublishError (Wombat::MamaTransport* tport, const char* topic, const void* platformInfo);
	void onTopicPublishErrorNotEntitled (Wombat::MamaTransport* tport, const char* topic, const void* platformInfo);
	void onTopicPublishErrorBadSymbol (Wombat::MamaTransport* tport, const char* topic, const void* platformInfo);

private:
	~MdsOmEnv();			// done by MdsOm

	/**
	 * Initialize the Env.
	 * @param config       The config for the env.
	 * @param enableStats  True to enable statistics collection
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	void init(const MdsOmConfig& config, bool enableStats);

	void load();

	void open();

	void close();

	void stopQueues();

	bool findSourceList(const char* source);
	bool handleDefaultSource() { return handlesDefaultSource; }

	Wombat::MamaQueueGroup* getQueueGroup(int size, mamaBridge bridge);
	void printQueueGroup(Wombat::MamaQueueGroup* g);

	void loadDictionary();
	void countTypes(mamaMsgType type, mamaMsgStatus status);
	void countOnQuality(mamaQuality quality);
	void loadSourceList();

	void createTransport(Wombat::MamaTransport* transport, const char* name);
	Wombat::MamaTransport* getTransport(const char* descr);
	int countTransport;

	void logPlatformInfo(Wombat::MamaTransport* t, const char* msg, const void* platformInfo) const;

	MdsOmEnvCollection sourceEnvList;

	const char* name;

	MdsOmEnvType type;

	const char* bridgeName;
	mamaBridge bridge;
	Wombat::MamaQueueGroup* queueGroup;

	MdsOmConfig config;
	MdsOmTransports transports;

	MdsOmTransportStatus dictionaryStatus;
	Wombat::MamaDictionary* dictionary;

	mamaPayloadType payloadId;			/**< the mamapayload id for the bridge */

	bool enableDq;
	bool requireInitial;
	bool handlesDefaultSource;

	wthread_mutex_t statsLock;

	// Stats vars ---------------------------------------------
	bool enableStats;
	mama_size_t countSubMsgs;
	mama_size_t countSubMsgsPeriod;
	mama_size_t countSubs;
	mama_size_t countPubs;
	mama_size_t countResponses;
	mama_size_t countPubMsgs;
	mama_size_t countPubMsgsPeriod;

	mama_size_t maxSubMsgRate;
	mama_size_t maxPubMsgRate;
	mama_size_t maxQueueCount;

	// Holds elapsed times for onMsg callback stats
	mama_f64_t btwnOnMsgSum;
	mama_f64_t inOnMsgSum;
	mama_size_t btwnOnMsgCount;
	mama_size_t inOnMsgCount;
	mama_f64_t maxInOnMsgAvg;
	mama_f64_t maxBtwnOnMsgAvg;

	// All of the msg types for stats counting
	mama_size_t countMAMA_MSG_TYPE_UPDATE;
	mama_size_t countMAMA_MSG_TYPE_INITIAL;
	mama_size_t countMAMA_MSG_TYPE_CANCEL;
	mama_size_t countMAMA_MSG_TYPE_ERROR;
	mama_size_t countMAMA_MSG_TYPE_CORRECTION;
	mama_size_t countMAMA_MSG_TYPE_CLOSING;
	mama_size_t countMAMA_MSG_TYPE_RECAP;
	mama_size_t countMAMA_MSG_TYPE_DELETE;
	mama_size_t countMAMA_MSG_TYPE_EXPIRE;
	mama_size_t countMAMA_MSG_TYPE_SNAPSHOT;
	mama_size_t countMAMA_MSG_TYPE_PREOPENING;
	mama_size_t countMAMA_MSG_TYPE_QUOTE;
	mama_size_t countMAMA_MSG_TYPE_TRADE;
	mama_size_t countMAMA_MSG_TYPE_ORDER;
	mama_size_t countMAMA_MSG_TYPE_BOOK_INITIAL;
	mama_size_t countMAMA_MSG_TYPE_BOOK_UPDATE;
	mama_size_t countMAMA_MSG_TYPE_BOOK_CLEAR;
	mama_size_t countMAMA_MSG_TYPE_BOOK_RECAP;
	mama_size_t countMAMA_MSG_TYPE_BOOK_SNAPSHOT;
	mama_size_t countMAMA_MSG_TYPE_NOT_PERMISSIONED;
	mama_size_t countMAMA_MSG_TYPE_NOT_FOUND;
	mama_size_t countMAMA_MSG_TYPE_END_OF_INITIALS;
	mama_size_t countMAMA_MSG_TYPE_WOMBAT_REQUEST;
	mama_size_t countMAMA_MSG_TYPE_WOMBAT_CALC;
	mama_size_t countMAMA_MSG_TYPE_SEC_STATUS;
	mama_size_t countMAMA_MSG_TYPE_DDICT_SNAPSHOT;
	mama_size_t countMAMA_MSG_TYPE_MISC;
	mama_size_t countMAMA_MSG_TYPE_TIBRV;
	mama_size_t countMAMA_MSG_TYPE_FEATURE_SET;
	mama_size_t countMAMA_MSG_TYPE_SYNC_REQUEST;
	mama_size_t countMAMA_MSG_TYPE_REFRESH;
	mama_size_t countMAMA_MSG_TYPE_WORLD_VIEW;
	mama_size_t countMAMA_MSG_TYPE_NEWS_QUERY;
	mama_size_t countMAMA_MSG_TYPE_NULL;

	// All of the status types for stats counting
	mama_size_t countMAMA_MSG_STATUS_OK;
	mama_size_t countMAMA_MSG_STATUS_LINE_DOWN;
	mama_size_t countMAMA_MSG_STATUS_NO_SUBSCRIBERS;
	mama_size_t countMAMA_MSG_STATUS_BAD_SYMBOL;
	mama_size_t countMAMA_MSG_STATUS_EXPIRED;
	mama_size_t countMAMA_MSG_STATUS_TIMEOUT;
	mama_size_t countMAMA_MSG_STATUS_MISC;
	mama_size_t countMAMA_MSG_STATUS_STALE;
	mama_size_t countMAMA_MSG_STATUS_PLATFORM_STATUS;
	mama_size_t countMAMA_MSG_STATUS_NOT_ENTITLED;
	mama_size_t countMAMA_MSG_STATUS_NOT_FOUND;
	mama_size_t countMAMA_MSG_STATUS_POSSIBLY_STALE;
	mama_size_t countMAMA_MSG_STATUS_NOT_PERMISSIONED;
	mama_size_t countMAMA_MSG_STATUS_TOPIC_CHANGE;
	mama_size_t countMAMA_MSG_STATUS_BANDWIDTH_EXCEEDED;
	mama_size_t countMAMA_MSG_STATUS_DUPLICATE;
	mama_size_t countMAMA_MSG_STATUS_UNKNOWN;
};

}

#endif
