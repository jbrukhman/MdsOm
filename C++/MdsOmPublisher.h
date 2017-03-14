#ifndef MdsOmPublisher_H
#define MdsOmPublisher_H

#include "MdsOm.h"

namespace MdsOmNs {

/**
 * Class for sending publisher feedback to apps.
 * Solace and Tick42 do this differently.
 * There is a workstream in OM to add that into OM (asof June 18, 2015).
 */
class MDSOMExpDLL MdsOmPublisherCallback {
public:
	MdsOmPublisherCallback();
	virtual ~MdsOmPublisherCallback();
	virtual void onPublishCreate(MdsOmPublisher* pub, void* closure) {};
	virtual void onPublishError(MdsOmPublisher* pub, const Wombat::MamaStatus& status, void* closure) {};
	virtual void onPublishDestroy(MdsOmPublisher* pub, void* closure) {};
};

/**
 * Class to handle Solace publisher feedback.
 * This is via a transport-specific subscription that the bridge manages.
 */
#ifdef OM_241
class MDSOMExpDLL MdsOmPublisherFeedback {
#else
class MDSOMExpDLL MdsOmPublisherFeedback : public Wombat::MamaBasicSubscriptionCallback {
#endif
public:
	MdsOmPublisherFeedback();
	virtual ~MdsOmPublisherFeedback();

#ifndef OM_241
	void addPublisher(MdsOmPublisher* p, const char* subject, Wombat::MamaTransport* transport, bool isSolace);
	void solacePublishFail(Wombat::MamaMsg& msg);

	// For MamaBasicSubscription
    void onCreate (
        Wombat::MamaBasicSubscription*  subscription);

    void onError (
        Wombat::MamaBasicSubscription*  subscription,
        const Wombat::MamaStatus&       status,
        const char*             topic);

    void onMsg (
        Wombat::MamaBasicSubscription*  subscription,
        Wombat::MamaMsg&                msg);

    void onDestroy (
        Wombat::MamaBasicSubscription* subscription,
        void*                  closure);
#else
	void addPublisher(MdsOmPublisher* p, const char* subject, Wombat::MamaTransport* transport);
#endif
	void removePublisher(MdsOmPublisher* p);

	void processTransportTopicPublishers(const char* subject, mama_status mamaStatus);

private:
	wthread_mutex_t feedbackLock;

	typedef set<MdsOmPublisher*> PublisherList;			// Set of publishers for a specific topic
	typedef map<string, PublisherList*> PublisherMap;	// Map of topic to set of publishers
	PublisherMap feedbackMap;
#ifndef OM_241
	set<Wombat::MamaTransport*> transportMap;
#endif

	void processPublishers(const char* subject, mama_status mamaStatus, const char* msg);
};

/**
 * Wrapper for MamaPublisher.
 *
 * This provides access to the MdsOmEnv and support for the async publisher callbacks.
 */
#ifdef OM_241
class MDSOMExpDLL MdsOmPublisher : public Wombat::MamaPublisher, public Wombat::MamaPublisherCallback {
#else
class MDSOMExpDLL MdsOmPublisher : public Wombat::MamaInboxCallback, public Wombat::MamaPublisher {
#endif
public:
	friend class MdsOm;
	friend class MdsOmEnv;
	friend class MdsOmPublisherFeedback;

	MdsOmPublisher();
	
	/**
	 * Create a publisher.
	 * @param              env the MdsOmEnv to create the publisher for.
	 * @param              transport the MamaTransport for the publisher.
	 * @param			   callback interface for publisher events (e.g., permission denied).
	 * @param              source the source (e.g., "IDN_DEV").
	 * @param              symbol the symbol (e.g., "AAPL.N").
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	void create(MdsOmEnv* env, Wombat::MamaTransport* transport, const char* source, const char* symbol, MdsOmPublisherCallback* cb = NULL, void* closure = NULL);

	/**
	 * Publish a message.
	 * This is asynchronous.
	 * There if publish feedback only if the MdsOmPublisherCallback is included in the create method.
	 * @param              msg the MamaMsg to publish.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	void send(Wombat::MamaMsg* msg) const;

	/**
	 * Destroy this publisher.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	void destroy(void);

	/**
	 * Get a new MamaMsg to publish.
	 * This method handles setting the correct payload id for this publisher.
	 * The msg is owned by the caller, but can be reused for the life of the application.
	 * @return             MamaMsg*.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	Wombat::MamaMsg* getNewMamaMsg() const;

	/**
	 * Destroy a MamaMsg.
	 * @param              msg the MamaMsg to destroy.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	void destroyMamaMsg(Wombat::MamaMsg* msg);

	/**
	 * Get the underlying MamaTransport.
	 * @return             MamaTransport.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	Wombat::MamaTransport* getMamaTransport() const;

	/**
	 * Get the MdsOmEnv for this publisher.
	 * @return             MdsOmEnv.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	MdsOmEnv* getEnv() const;

	/**
	 * Get the MamaDictionary.
	 * @return             MamaDictionary.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	Wombat::MamaDictionary* getDictionary() const;

#ifndef OM_241
	const char* getSymbol() const;
	const char* getSource() const;
#endif
	const char* getSubject() const;

	/**
	 * Store a MamaMsg in the publisher. Useful for reusing MamaMsgs.
	 * @param              msg the MamaMsg to store.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	void storeMamaMsg(Wombat::MamaMsg* msg);

	/**
	 * Retrieve a stored MamaMsg in the publisher. Useful for reusing MamaMsgs.
	 * @return             MamaMsg.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	Wombat::MamaMsg* retrieveMamaMsg() const;

	MdsOmPublisherCallback* getCb() const;

#ifndef OM_241
	// RMDS INBOX
    void onDestroy(Wombat::MamaInbox *inbox, 
                   void *closure);

    void onMsg(Wombat::MamaInbox* inbox,
               Wombat::MamaMsg& msg);

    void onError(Wombat::MamaInbox* inbox,
                 const Wombat::MamaStatus& status);
#endif

	void* getClosure() const;
	void setClosure(void* closure);

	bool isValid() const;
	void setValid(bool valid);

#ifdef OM_241
	// For OpenMAMA MamaPublisherCallback
	void onCreate (
		Wombat::MamaPublisher* publisher,
		void* closure);

	void onDestroy (
		Wombat::MamaPublisher* publisher,
		void* closure);

	void onError (
		Wombat::MamaPublisher* publisher,
		const Wombat::MamaStatus& status,
		const char* info,
		void* closure);
#endif

private:
	virtual ~MdsOmPublisher();			// app must delegate to destroy()

#ifndef OM_241
	// Temp for Tick42
	Wombat::MamaInbox* inbox;
#endif

	static MdsOmPublisherFeedback feedback;

	std::string subject;
#ifndef OM_241
	std::string symbol;
	std::string source;
#endif

	Wombat::MamaTransport* transport;
	MdsOmEnv* env;
	Wombat::MamaMsg* msg;

	MdsOmPublisherCallback* cb;
	void* closure;
	bool valid;					// used by apps to track validity
};

}

#endif

