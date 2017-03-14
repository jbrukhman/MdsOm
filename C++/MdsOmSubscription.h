#ifndef MdsOmSubscription_H
#define MdsOmSubscription_H

#include "MdsOm.h"

namespace MdsOmNs {

/**
 * Wrapper for Wombat::MamaSubscription.
 *
 * This provides access to the MdsOmEnv on subscription callbacks.
 * The Wombat::MamaSubscription* can be cast to this in callbacks.
 */
class MDSOMExpDLL MdsOmSubscription : public Wombat::MamaSubscription {
private:
	class MDSOMExpDLL MdsOmSubscriptionCb : public Wombat::MamaSubscriptionCallback {
	public:
		MdsOmSubscriptionCb(MdsOmSubscription& parent) : parent(parent) {}
		virtual ~MdsOmSubscriptionCb() {}

	public:
		// Wombat::MamaSubscriptionCallback
		/** @see Wombat::MamaSubscriptionCallback */
		virtual void onCreate(Wombat::MamaSubscription* subscription);

		/** @see Wombat::MamaSubscriptionCallback */
		virtual void onError(Wombat::MamaSubscription* subscription, const Wombat::MamaStatus& status, const char* symbol);

		/** @see Wombat::MamaSubscriptionCallback */
		virtual void onGap(Wombat::MamaSubscription* subscription);

		/** @see Wombat::MamaSubscriptionCallback */
		virtual void onDestroy(Wombat::MamaSubscription* subscription);

		/** @see Wombat::MamaSubscriptionCallback */
		virtual void onRecapRequest(Wombat::MamaSubscription* subscription);

		/** @see Wombat::MamaSubscriptionCallback */
		virtual void onMsg(Wombat::MamaSubscription* subscription, Wombat::MamaMsg& msg);

		/** @see Wombat::MamaSubscriptionCallback */
		virtual void onQuality(Wombat::MamaSubscription* subscription, mamaQuality quality, const char* symbol, short cause, const void* platformInfo);
	private:
		MdsOmSubscription& parent;
	};

public:
	friend class MdsOmEnv;

	/**
	 * Subscribe to a symbol.
	 * @param env      The MdsOmEnv*.
	 * @param source   The source, e.g., IDN_DEV or PB or cBPOD_DEV
	 * @param symbol   The symbol, this is just the symbol, e.g., like TWTR.N
	 * @param cb       The interface to send callbacks to.
	 * @param closure  The closure to send with each callback, can be NULL.
	 * @param setup    If true then call Wombat::MamaSubscription setup instead of create, this requires later calling activate() to make the sub startup.
	 * @return         The subscription object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void create(MdsOmEnv* env, Wombat::MamaTransport* transport, const char* source, const char* symbol, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup);
	
	/**
	 * Snapshot a symbol, only the Initial is returned. No unsubscribe is required.
	 * @param env      The MdsOmEnv*.
	 * @param source   The source, e.g., IDN_DEV or PB or cBPOD_DEV
	 * @param symbol   The symbol, this is just the symbol, e.g., like TWTR.N
	 * @param cb       The interface to send callbacks to.
	 * @param closure  The closure to send with each callback, can be NULL.
	 * @param setup    If true then call Wombat::MamaSubscription setup instead of create, this requires later calling activate() to make the sub startup.
	 * @return         The subscription object.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void snap(MdsOmEnv* env, Wombat::MamaTransport* transport, const char* source, const char* symbol, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup);
	
	/**
	 * Get the MdsOmEnv that this subscription is active on.
	 * @return             MdsOmEnv instance.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	MdsOmEnv* getEnv() const { return env; }

	/**
	 * Get the dictionary that is active for this subscription.
	 * @return             MamaDictionary instance.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	Wombat::MamaDictionary* getDictionary() const;

	/**
	 * Unsubscribe.
	 * This returns after calling MamaSubscription::destroy().
	 * The obj is deleted in the onDestroy callback.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void destroy();

	/**
	 * Unsubscribe.
	 * This method waits, via semaphone, for the onDestroy to finish.
	 * This can NOT be called from the thread that handles the callbacks (e.g., onMsg, onErrom etc).
	 * If you want to call destroy from the callback thread use the version above.
	 * @param		interval - wait time in milliseconds for semaphore. 0=no timer, +ve=milliseconds to wait
	 @ @return		from semaphore wait, 0=ok, -1=error (most likely timed-out).
	 * @throws         MamaStatus, MdsOmStatus
	 */
	int destroy(size_t interval);

	/**
	 * Unsubscribe.
	 * This returns after calling MamaSubscription::destroyEx().
	 * DestroyEx() places the request on the subscription's queue, and the processes it from the thread that dispatches the queue.
	 * The obj is deleted in the onDestroy callback.
	 * @throws         MamaStatus, MdsOmStatus
	 */
	void destroyEx();

	/**
	 * Unsubscribe.
	 * DestroyEx() places the request on the subscription's queue, and the processes it from the thread that dispatches the queue.
	 * This method waits, via semaphone, for the onDestroy to finish.
	 * This can NOT be called from the thread that handles the callbacks (e.g., onMsg, onErrom etc).
	 * If you want to call destroyEx from the callback thread use the version above.
	 * @param		interval - wait time in milliseconds for semaphore. 0=no timer, +ve=milliseconds to wait
	 @ @return		from semaphore wait, 0=ok, -1=error (most likely timed-out).
	 * @throws         MamaStatus, MdsOmStatus
	 */
	int destroyEx(size_t interval);

	/**
	 * Return the number of msgs received for this subscription.
	 * @return			number of messages.
	 */
	long getMsgs() const { return msgs; }

private:
	void createInternal(MdsOmEnv* env, Wombat::MamaTransport* transport, const char* source, const char* symbol, Wombat::MamaSubscriptionCallback* cb, void* closure, bool setup, mamaServiceLevel subType);

	MdsOmSubscription();

	int internalDestroy(bool useSem, size_t interval, bool useEx);

	// This should be called via the destroy methods, which manage the lifecycle
	~MdsOmSubscription();

	MdsOmEnv* env;

	wsem_t sem;

	Wombat::MamaSubscriptionCallback* cb;	// the mama subscription callback
	MdsOmSubscriptionCb localCb;	// local callback that then calls to the app
	bool gotResponse;				// got 1+ msgs from the app
	bool waitingForDestruct;		// been destructed, waiting for queued msg
	bool deleteInOnDestroy;			// delete obj in onDestroy
	long msgs;						// msgs received
};

}

#endif
