/** 
 * @file MdsOmChain.h
 * @brief Single include file for MdsOm.
 * @see <twiki>
 */

#ifndef MdsOmLinkCallback_H
#define MdsOmLinkCallback_H

#include "MdsOmChains.h"

namespace MdsOmNs {

class MDSOMExpDLL MdsOmLinkCallback : public MamaSubscriptionCallback
{
public:
	MdsOmLinkCallback();
	~MdsOmLinkCallback(); 

	// methods that mama needs
	virtual void onCreate(MamaSubscription* subscriber);
	virtual void onError(MamaSubscription* subscription, const MamaStatus& status, const char* subject);
	virtual void onMsg(MamaSubscription* subscription, MamaMsg& msg);
	virtual void onQuality(MamaSubscription* subscription, mamaQuality quality, const char* symbol, short cause, const void* platformInfo); 
	virtual void onGap(MamaSubscription* subscription);
	virtual void onRecapRequest(MamaSubscription* subscription);

	bool GetNext(MamaMsg& msg, MamaSubscription* subscription, ChainType ct);
	ChainType MdsOmLinkCallback::CheckForChain(MamaMsg& msg, MamaSubscription* subscription);

	MamaDictionary* dictionary;
	string source;
};

}

#endif
