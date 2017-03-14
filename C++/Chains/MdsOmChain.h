/** 
 * @file MdsOmChain.h
 * @brief Single include file for MdsOm.
 * @see <twiki>
 */

#ifndef MdsOmChain_H
#define MdsOmChain_H

#include "MdsOmChains.h"

namespace MdsOmNs {

class MDSOMExpDLL MdsOmChain : public MdsOmPubChain, public MdsOmSubChain
{
public:
	friend class MdsOmLink;

	MdsOmChain(MdsOm* om, const char* subject, const char* templateName);

	MdsOmChain(MdsOm* om, const char* source, const char* symbol, const char* templateName);

	virtual ~MdsOmChain(); 

	void destroy();

	void setPublisherCallback(MdsOmPublisherCallback* cb, void* closure);
	MdsOmPublisherCallback* getPublisherCallback() const;
	void* getPublisherClosure() const;

	// Subscribe and unsubscribe
	void subscribe(MdsOmChainCallback* cb, void* closure);
	void unsubscribe();

	// MDSChain Capture 
	void capture(MdsOmChainCallback* cb, void* closure);

	// Get all elements
	size_t getElementCount() const;
	size_t getPubElementCount() const;
	size_t getSubElementCount() const;
	MdsOmList<const char*>* getElements(MdsOmList<const char*>* l) const;

	size_t getLinkCount() const;
	MdsOmList<const char*>* getLinkNames(MdsOmList<const char*>* l) const;

	MdsOmList<MdsOmDataLink*>* getLinks(MdsOmList<MdsOmDataLink*>* l) const;

	// Get the seconds to return the entire chain
	time_t getTime() const { return elapsedTime; }


	//--------------------------------------------------------------------------
	// Maintain elements.
	// The index is 1-based.
	//--------------------------------------------------------------------------
	void addElement(const char* elementName);
	void insertElement(int indx, const char *pszElementName);
	void removeElement(int indx);
	void modifyElement(int indx, const char *pszElementName);
	void clearChain();

	//--------------------------------------------------------------------------
	// Publish the chain
	//--------------------------------------------------------------------------
	void publish();
	void publishAll();

	void publishOne();
	void publishOneAll();
	void publishTwo();

	//--------------------------------------------------------------------------
	// Drop the chain
	//--------------------------------------------------------------------------
	void drop();

	//--------------------------------------------------------------------------
	// Get the chain name
	//--------------------------------------------------------------------------
	const char* getChainName() const;
	const char* getSymbol() const;
	const char* getSource() const;
	const char* getSubject() const;

	bool getChainDoneSub() const;

private:
	void processNextLink(MdsOmLink* link);
	void errorNextLink(MdsOmLink* link, MdsOmStatusCode status, const char* subject);
	void onQualityLink(MdsOmLink* link, mamaQuality quality, const char* subject);

	void subscribeBase();

	void subscribeLink(const char* source, const char *symbol);
	MdsOmLink* addLink(const char* source, const char* symbol);

	void init();

	void clearElements();
	void clearLinks();
	void addElement2(MdsOmElement* e);
	void removeLinksAfter(MdsOmLink* link);
	bool isLastLink(MdsOmLink* link);
	void getConfig();

	MdsOm* om;
	Wombat::MamaDictionary* dictionary;

	string templateName;
	ChainConfig* chainConfig;

	// somewhere to store the LINKS
	LinkList linkList; 
	ElementList elementList;			// store elements for pub
	mutable wthread_mutex_t listLock;

	MdsOmPublisherCallback* pubCb;
	void* pubClosure;

	MdsOmChainCallback* cb;
	void* closure;

	string source;
	string symbol;
	string subject;

	mamaServiceLevel subType; //Used to hold whether it is SNAPSHOT or REALTIME.
	bool firstPublish;
	bool chainDoneSub;

	bool destroyed;

	time_t startTime;
	time_t elapsedTime;
};

}

#endif

