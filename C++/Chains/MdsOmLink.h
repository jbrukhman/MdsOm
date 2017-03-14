//******************************************************************************
// Define MDSLink class and friendly MDSElement class.
//******************************************************************************

#ifndef MdsOmLink_H
#define MdsOmLink_H

#include "MdsOmChains.h"

namespace MdsOmNs {

//******************************************************************************
// MDSLink class
//******************************************************************************
class MdsOmLink : public MdsOmDataLink, public Wombat::MamaSubscriptionCallback 
{
public:
	//--------------------------------------------------------------------------
	// Constructor/destructor
	//--------------------------------------------------------------------------
	MdsOmLink();

	void destroy();

	//--------------------------------------------------------------------------
	// Initialise the link
	//--------------------------------------------------------------------------
	int init(MdsOmChain* chain, ChainConfig* chainConf, const char *source, const char *symbol);

	//---------------------------------------------------------------------------
	// Subscribe to the link(and unsubscribe)
	//--------------------------------------------------------------------------
	void subscribe();
	void unsubscribe();
	void setUpCapture();
	void capture();

	Wombat::MamaMsg* getSubMamaMsg() const;
	mamaMsgType getSubMamaMsgType() const { return msgType; }
	Wombat::MamaMsg* getPubMamaMsg() const;

	// Link Type
	MdsOmChainLinkType getLinkType() const;
	void setLinkType(MdsOmChainLinkType t);
	const char* getChainLinkTypeText(MdsOmChainLinkType t) const;

	//--------------------------------------------------------------------------
	// Drop the link
	//--------------------------------------------------------------------------
	void drop();

	//--------------------------------------------------------------------------
	// Set and get the status of the link in the chain
	//--------------------------------------------------------------------------
	MdsOmStatusCode getStatus() const;

	//--------------------------------------------------------------------------
	// Functions to iterate through the element names
	//--------------------------------------------------------------------------
	MdsOmList<const char*>* getElements(MdsOmList<const char*>* l);

	//--------------------------------------------------------------------------
	// Add an element to the link. Returns a +ve value if the addition fails.
	// This will be due to the maximum number of elements already being in
	// the link
	//--------------------------------------------------------------------------
	bool addElement(const char* name);

	//--------------------------------------------------------------------------
	// Check to see if the link is already full.
	//--------------------------------------------------------------------------
	bool isLinkFull() const;

	//--------------------------------------------------------------------------
	// Get the number of elements in the link
	//--------------------------------------------------------------------------
	size_t	getElementCount() const;

	//--------------------------------------------------------------------------
	// Get the maximum number of elements per link.
	//--------------------------------------------------------------------------
	size_t	getMaxElements() const;

	//--------------------------------------------------------------------------
	// Get the number of elements in the link
	//--------------------------------------------------------------------------
	size_t	getNumUpdatesSinceCB() const;
	size_t	getTotalNumUpdates() const;

	MdsOmChain* getChain() const;

	//--------------------------------------------------------------------------
	// Get the name of the link
	//--------------------------------------------------------------------------
	const char* getSubject() const;
	const char* getSymbol() const;
	const char* getSource() const;

	// methods that mama needs
	virtual void onCreate(Wombat::MamaSubscription* subscriber);
	virtual void onError(Wombat::MamaSubscription* subscription, const Wombat::MamaStatus& status, const char* subject);
	virtual void onMsg(Wombat::MamaSubscription* subscription, Wombat::MamaMsg& msg);
	virtual void onQuality(Wombat::MamaSubscription* subscription, mamaQuality quality, const char* symbol, short cause, const void* platformInfo); 
	virtual void onGap(Wombat::MamaSubscription* subscription);
	virtual void onRecapRequest(Wombat::MamaSubscription* subscription);
	virtual void onDestroy(Wombat::MamaSubscription* subscription);

	bool getNext(Wombat::MamaMsg& msg, Wombat::MamaSubscription* subscription, MdsOmChainLinkType ct);
	void checkForChain(Wombat::MamaMsg& msg, Wombat::MamaSubscription* subscription);

	bool getDirty() const { return dirty; }
	void setDirty(bool dirty) { this->dirty = dirty; }

	//--------------------------------------------------------------------------
	// Publish the link
	//--------------------------------------------------------------------------
	void publish(bool firstPublish);

	// Used to set other fields into links
	void publishOne(bool firstPublish);
	void publishTwo(bool firstPublish);

	const char* getNextLinkName() const { return nextLinkName.c_str(); }
	void setNextLinkName(const char *pszNextLinkName);
	void setPrevLinkName(const char *pszPrevLinkName);

	void clearElements();

	ElementList& getElementList() { return elementList; }

private:
	virtual ~MdsOmLink();

	MdsOmEnv* env;
	MdsOmChain* theChain;
	MdsOmSubscription* sub;
	MdsOmPublisher* pub;

	Wombat::MamaMsg* subMsg;

	Wombat::MamaDictionary* dictionary;
	ChainConfig* chainConfig;

	// List of elements
	wthread_mutex_t elementListLock;
	ElementList elementList;

	int _nNumUpdates;			// Total number of updates received since start of subscription
	MdsOmStatusCode status;		// Status of the link, starts life OK
	
	string source;
	string symbol;
	string subject;

	//--------------------------------------------------------------------------
	// link names
	//--------------------------------------------------------------------------
	string nextLinkName;
	string prevLinkName;

	void displayMsg(MdsOmEnv* env, const char* source, const char* symbol, const Wombat::MamaMsg& msg, int indent);
	const char* getFieldName(MdsOmEnv* env, Wombat::MamaMsgField field, mama_fid_t fid);
	Wombat::MamaMsg* setupPub();

	mamaMsgType msgType;

	MdsOmChainLinkType linkType;

	//--------------------------------------------------------------------------
	// Field specfies if any elements have been added/deleted/modified from the link
	// since the last publish call.
	//--------------------------------------------------------------------------
	mutable bool dirty;
};

}

#endif
