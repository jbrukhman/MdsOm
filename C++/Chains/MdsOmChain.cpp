//
// MdsOmChain
// This is the chain helper class for OpenMAMA
//
#pragma warning(disable : 4996)

#include "MdsOmChains.h"

namespace MdsOmNs {

MdsOmChain::MdsOmChain(MdsOm* om, const char* source, const char* symbol, const char* templateName)
{
	this->om = om;
	if (templateName) this->templateName = templateName;
	if (source) this->source = source;
	if (symbol) this->symbol = symbol;

	init();
}

MdsOmChain::MdsOmChain(MdsOm* om, const char* subject, const char* templateName)
{
	this->om = om;
	if (templateName) this->templateName = templateName;
	if (subject) {
		pair<string, string> tokens = getSourceAndSymbol(subject);
		this->source = tokens.first;
		this->symbol = tokens.second;
	}

	init();
}

// This is private, the app uses destroy()
MdsOmChain::~MdsOmChain()
{
	if (chainConfig) delete chainConfig;
	wthread_mutex_destroy(&listLock);
}

void MdsOmChain::destroy()
{
	destroyed = true;
	{
		MdsOmLock lock(&listLock);

		// Delete the links
		LinkList::iterator it = linkList.begin();
		while (it != linkList.end()) {
			MdsOmLink* link = *it++;
			link->destroy();
		}
		linkList.clear();

		// Delete the elements
		ElementList::iterator itx = elementList.begin();
		while (itx != elementList.end()) {
			MdsOmElement* e = *itx++;
			delete e;
		}
		elementList.clear();
	}

	delete this;			// last thing done
}

/**
 * Drop the links in the infrastructure.
 */
void MdsOmChain::drop(void)
{
	MdsOmLock lock(&listLock);

	if (linkList.size() == 0) {
		// Add a link to delete the first link.
		// This may not delete all of the links for the chain.
		MdsOmLink* link = addLink(source.c_str(), symbol.c_str());
	}

	LinkList::iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it++;
		link->drop();
	}

	clearChain();
}

void MdsOmChain::init()
{
	wthread_mutexattr_t attr;
	wthread_mutexattr_init (&attr);
	wthread_mutexattr_settype (&attr, WTHREAD_MUTEX_RECURSIVE);
	wthread_mutex_init(&listLock, &attr);

	destroyed = false;

	// Validate name
	const char* cp = symbol.c_str();
	if (!isdigit(*cp)) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmChain::Init: missing digit in chain name '%s'", symbol.c_str());
		throw MdsOmStatus(MDS_OM_STATUS_CHAIN_BAD_FORMAT);
	}

	cp = strchr(symbol.c_str(), '#');
	if (cp == NULL) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmChain::Init: missing '#' in chain name '%s'", symbol.c_str());
		throw MdsOmStatus(MDS_OM_STATUS_CHAIN_BAD_FORMAT);
	}

	firstPublish = true;
	elapsedTime = startTime = 0;
	pubCb = NULL;
	cb = NULL;
	closure = NULL;
	chainDoneSub = false;

	subject = source + "." + symbol;
	chainConfig = NULL;

	getConfig();
}

// This is called by the app when it wants publisher feedback
void MdsOmChain::setPublisherCallback(MdsOmPublisherCallback* cb, void* closure) {	pubCb = cb; pubClosure = closure; }

MdsOmPublisherCallback* MdsOmChain::getPublisherCallback() const { return pubCb; }

void* MdsOmChain::getPublisherClosure() const { return pubClosure; }

const char* MdsOmChain::getChainName() const { return subject.c_str(); }

const char* MdsOmChain::getSubject() const { return subject.c_str(); }

const char* MdsOmChain::getSymbol()	const { return symbol.c_str(); }

const char* MdsOmChain::getSource()	const { return source.c_str(); }

bool MdsOmChain::getChainDoneSub() const { return chainDoneSub; }

void MdsOmChain::getConfig()
{
	chainConfig = new ChainConfig(templateName.c_str());
	chainConfig->Init();
}

void MdsOmChain::subscribe(MdsOmChainCallback* cb, void* closure)
{
	this-> cb = cb;
	this->closure = closure;

	subType = MAMA_SERVICE_LEVEL_REAL_TIME;
	chainDoneSub = false;

	mama_log(MAMA_LOG_LEVEL_FINE, "Chain::Subscribe: %s.%s", source.c_str(), symbol.c_str());

	if (!om || source.size() == 0 || symbol.size() == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmChain::Subscribe: NULL parms om=%p source='%s' symbol='%s'", om, source.c_str(), symbol.c_str());
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	time(&startTime);

	subscribeLink(source.c_str(), symbol.c_str());
}

// Called on the init and for each subsequent link
void MdsOmChain::subscribeLink(const char* source, const char *symbol)
{
	MdsOmLink* link = addLink(source, symbol);

	switch (subType) {
		case MAMA_SERVICE_LEVEL_SNAPSHOT:
			link->capture();
			break;
		case MAMA_SERVICE_LEVEL_REAL_TIME:
		default:
			link->subscribe();
			break;
	}
}

// Add a new link to the chain
MdsOmLink* MdsOmChain::addLink(const char* source, const char* symbol)
{
	MdsOmLink* link = new MdsOmLink();
	link->init(this, chainConfig, source, symbol);
	{
		MdsOmLock lock(&listLock);
		linkList.push_back(link);
	}
	return link;
}

void MdsOmChain::unsubscribe()
{
	// This removes all links and elements
	clearChain();
}

void MdsOmChain::capture(MdsOmChainCallback* cb, void* closure)
{
	this-> cb = cb;
	this->closure = closure;

	subType = MAMA_SERVICE_LEVEL_SNAPSHOT;
	chainDoneSub = false;

	mama_log(MAMA_LOG_LEVEL_FINE, "CaptureChainsSubscribe: %s.%s", source.c_str(), symbol.c_str());

	if (!om || source.size() == 0 || symbol.size() == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmChain::Subscribe: NULL parms om=%p source='%s' symbol='%s'", om, source.c_str(), symbol.c_str());
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	// Clear the chain before capture, since they can call capture many times on a single chain
	clearChain();

	time(&startTime);

	// Subscribe to first link in chain
	subscribeLink(source.c_str(), symbol.c_str());
}

// This is called by the link when it finishes processing its msg
void MdsOmChain::processNextLink(MdsOmLink* link)
{
	if (destroyed == true) return;

	const char* nextLinkName = link->getNextLinkName();

	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmChain::ProcessNextLink: %s '%s'", source.c_str(), nextLinkName);

	if (strlen(nextLinkName) == 0) {
		// All done with chain, call the app
		time_t end;
		time(&end);
		elapsedTime = end - startTime;

		// Delete any remaining links
		removeLinksAfter(link);

		if (cb) cb->mdsOmChainOnLink(this, link, closure);

		// Only call chain callback when done and do it only once
		// Later link updates will send link cb, but not chain cb
		if (cb && !chainDoneSub) cb->mdsOmChainOnSuccess(this, closure);

		chainDoneSub = true;
	} else {
		// Subscribe to next link
		bool isLast = isLastLink(link);
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmChain::ProcessNextLink: %s '%s' last=%d", source.c_str(), nextLinkName, isLast);
		if (cb) cb->mdsOmChainOnLink(this, link, closure);
		if (isLast) {
			// Last link in our list, and we have a next name, so keep subscribing
			subscribeLink(source.c_str(), nextLinkName);
		}
	}
}

// This is called by the link when it finishes processing its msg with an error
void MdsOmChain::errorNextLink(MdsOmLink* link, MdsOmStatusCode code, const char* subject)
{
	if (destroyed == true) return;

	time_t end;
	time(&end);
	elapsedTime = end - startTime;

	// Error with the chain, call the app
	MdsOmStatus mstatus(code);
	if (cb) cb->mdsOmChainOnError(this, link, mstatus, closure);
	chainDoneSub = true;
}

// This is called by the link when it finishes processing its msg with an error
void MdsOmChain::onQualityLink(MdsOmLink* link, mamaQuality quality, const char* subject)
{
	if (destroyed == true) return;

	if (cb) cb->mdsOmChainOnLinkQuality(this, link, quality, closure);
}

// Is this the last link in the chain?
bool MdsOmChain::isLastLink(MdsOmLink* link)
{
	return link == linkList.back();
}

// Used to remove links after a link with no next field
void MdsOmChain::removeLinksAfter(MdsOmLink* after)
{
	MdsOmLock lock(&listLock);
	bool removing = false;
	LinkList::iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it;
		if (removing) {
			link->destroy();
			it = linkList.erase(it);
		} else {
			it++;
		}
		if (link == after) {
			removing = true;
		}
	}
}

size_t MdsOmChain::getLinkCount() const
{
	return linkList.size();
}

size_t MdsOmChain::getPubElementCount() const
{
	return elementList.size();
}

size_t MdsOmChain::getSubElementCount() const
{
	MdsOmLock lock(&listLock);
	size_t count = 0;
	LinkList::const_iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it++;
		count += link->getElementCount();
	}
	return count;
}

size_t MdsOmChain::getElementCount() const
{
	return getSubElementCount();
}

MdsOmList<const char*>* MdsOmChain::getElements(MdsOmList<const char*>* l) const
{
	MdsOmLock lock(&listLock);

	// This will return all of the elements of elementList
	if (l == NULL) l = new MdsOmList<const char*>();
	else l->clear();
	LinkList::const_iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it++;
		ElementList::iterator ite = link->getElementList().begin();
		while (ite != link->getElementList().end()) {
			MdsOmElement* e = *ite++;
			l->add(e->getName());
		}
	}
	return l;
}

/** Get all of the link names(subjects)
 */
MdsOmList<const char*>* MdsOmChain::getLinkNames(MdsOmList<const char*>* l) const
{
	MdsOmLock lock(&listLock);

	// This will return all of the linknames
	if (l == NULL) l = new MdsOmList<const char*>();
	else l->clear();
	LinkList::const_iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it++;
		l->add(link->getSubject());
	}
	return l;
}

/** Get all of the link objects.
*/
MdsOmList<MdsOmDataLink*>* MdsOmChain::getLinks(MdsOmList<MdsOmDataLink*>* l) const
{
	MdsOmLock lock(&listLock);

	// This will return all of the linknames
	if (l == NULL) l = new MdsOmList<MdsOmDataLink*>();
	else l->clear();
	LinkList::const_iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmDataLink* link = *it++;
		l->add(link);
	}
	return l;
}

//******************************************************************************
// Add an element to the chain at the end
//******************************************************************************
void MdsOmChain::addElement(const char *name)
{
	MdsOmLock lock(&listLock);

	// Insert at end of elementList
	if (name == NULL) name = "";

	MdsOmElement* e = new MdsOmElement();
	e->init(name);

	elementList.push_back(e);
}

//******************************************************************************
// Delete an element from the chain at position indx(1-based)
//******************************************************************************
void MdsOmChain::removeElement(int indx)
{
	MdsOmLock lock(&listLock);

	// The index is 1-based
	if (indx <= 0 || elementList.size() == 0 || elementList.size() < (size_t) indx) throw MdsOmStatus(MDS_OM_STATUS_CHAIN_BAD_INDEX);
	indx--;

	ElementList::iterator it = elementList.begin();
	std::advance(it, indx);			// erases at current
	it = elementList.erase(it);
	if (it != elementList.begin()) it--;		// set to previous to make it dirty, that allows the link to be published correctly

	// Set the rest as dirty
	while (it != elementList.end()) {
		MdsOmElement* e = *it++;
		e->setDirty(true);
	}
}

//******************************************************************************
// Update an element in the chain at position indx(1-based)
//******************************************************************************
void MdsOmChain::insertElement(int indx, const char *name)
{
	MdsOmLock lock(&listLock);

	// The index is 1-based
	if (indx <= 0 || elementList.size() == 0 || elementList.size() <(size_t) indx) throw MdsOmStatus(MDS_OM_STATUS_CHAIN_BAD_INDEX);
	indx--;

	if (name == NULL) name = "";

	ElementList::iterator it = elementList.begin();
	std::advance(it, indx);				// inserts before current
	MdsOmElement* e = new MdsOmElement();
	e->init(name);
	it = elementList.insert(it, e);

	// Set the rest as dirty
	while (it != elementList.end()) {
		MdsOmElement* e = *it++;
		e->setDirty(true);
	}
}

//******************************************************************************
// Update an element in the chain at position indx(1-based)
//******************************************************************************
void MdsOmChain::modifyElement(int indx, const char *pszElementName)
{
	MdsOmLock lock(&listLock);

	// The index is 1-based
	if (indx <= 0 || elementList.size() == 0 || elementList.size() <(size_t) indx) throw MdsOmStatus(MDS_OM_STATUS_CHAIN_BAD_INDEX);
	indx--;

	ElementList::iterator it = elementList.begin();
	std::advance(it, indx);					// modify at current
	MdsOmElement* e = *it;
	e->init(pszElementName);
	e->setDirty(true);
}

void MdsOmChain::clearChain()
{
	clearLinks();
	clearElements();
}

void MdsOmChain::clearElements()
{
	MdsOmLock lock(&listLock);

	ElementList::iterator it = elementList.begin();
	while (it != elementList.end()) {
		MdsOmElement* e = *it++;
		delete e;
	}
	elementList.clear();
}

void MdsOmChain::clearLinks()
{
	MdsOmLock lock(&listLock);

	LinkList::iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it++;
		link->destroy();
	}
	linkList.clear();
}

/** Used when creating links before a publish.
 * The elements are kept in elementList, and then the links are
 * created from that just before a publish.
 */
void MdsOmChain::addElement2(MdsOmElement* e)
{
	MdsOmLock lock(&listLock);

	MdsOmLink* link;
	if (linkList.size() == 0) {
		// No links at all, add first one with chain subject
		link = addLink(source.c_str(), symbol.c_str());
	} else {
		// Get the end of the list
		link = linkList.back();
	}

	bool ret = link->addElement(e->getName());
	if (ret) {
		if (e->getDirty()) link->setDirty(true);
	} else {
		// Last link was full, add another one
		// Get next link name
		MdsOmChainName chainName;
		char next[MAX_STR_TEK_SUBJECT+1];
		chainName.init(source.c_str(), link->getSymbol(), chainConfig);
		chainName.getNextLinkSubject(next, MAX_STR_TEK_SUBJECT);

		MdsOmLink* newLink = addLink(source.c_str(), next);
		ret = newLink->addElement(e->getName());				// this sets the link dirty if the element is dirty
		if (e->getDirty()) newLink->setDirty(true);
		if (e->getDirty()) link->setDirty(true);	// set prev link dirty to publish it, since it contains the link to this new link
	}
}

/**
 * Publish a chain.
 * This will create the links based on the elements in elementList.
 */
void MdsOmChain::publish()
{
	publishOne();
	publishTwo();
}

/**
 * Publish ALL of a chain to refresh.
 * This will create the links based on the elements in elementList.
 */
void MdsOmChain::publishAll()
{
	MdsOmLock lock(&listLock);
	ElementList::iterator it = elementList.begin();
	while (it != elementList.end()) {
		MdsOmElement* e = *it++;
		e->setDirty(true);
	}
	publish();
}

void MdsOmChain::publishOneAll()
{
	MdsOmLock lock(&listLock);
	ElementList::iterator it = elementList.begin();
	while (it != elementList.end()) {
		MdsOmElement* e = *it++;
		e->setDirty(true);
	}
	publishOne();
}

/** This is only used by an app that wants to add extra fields.
 * To do that call PublishOne(), the get the MamaMsg and add fields, and then call PublishTwo().
 */
void MdsOmChain::publishOne()
{
	MdsOmLock lock(&listLock);

	if (elementList.size() == 0) {
		// Empty list, publish a single link with empty elements
		clearLinks();
		MdsOmLink* link = addLink(source.c_str(), symbol.c_str());
		link->setDirty(true);
	} else {
		// Remove all links and reload
		// This clears all of the links. I tried only clearing the links after the first dirty element,
		//  but the savings in time was only a few milliseconds per chain publish.
		// If an app has latency issues with chains see if the can batch the changes and then publish once.
		clearLinks();
		ElementList::iterator ite = elementList.begin();
		while (ite != elementList.end()) {
			MdsOmElement* e = *ite++;
			addElement2(e);
		}
	}

	// Set prev/next names
	string prevName;
	MdsOmLink* prev = NULL;
	LinkList::iterator it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it++;
		link->setPrevLinkName(prevName.c_str());
		link->setNextLinkName("");				// to get last one
		prevName = link->getSymbol();
		if (prev) prev->setNextLinkName(prevName.c_str());
		prev = link;
	}

	// Mark all elements as published
	ElementList::iterator ite = elementList.begin();
	while (ite != elementList.end()) {
		MdsOmElement* e = *ite++;
		e->setDirty(false);
	}

	// Now publish One
	it = linkList.begin();
	while (it != linkList.end()) {
		MdsOmLink* link = *it++;
		link->publishOne(firstPublish);
	}
}

void MdsOmChain::publishTwo()
{
	MdsOmLock lock(&listLock);

	// Now publish.
	if (chainConfig->getReversePublish() == false) {
		LinkList::iterator it = linkList.begin();
		while (it != linkList.end()) {
			MdsOmLink* link = *it++;
			if (link->getDirty()) {
				link->publishTwo(firstPublish);
			}
		}
	} else {
		// Publish end to start so that an app does not get a next link that has not been published yet
		// Will prevent a not_found race condition for a subscriber.
		LinkList::reverse_iterator it = linkList.rbegin();
		while (it != linkList.rend()) {
			MdsOmLink* link = *it++;
			if (link->getDirty()) {
				link->publishTwo(firstPublish);
			}
		}
	}

	firstPublish = false;
}

}
