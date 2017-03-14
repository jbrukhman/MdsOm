#ifndef MdsOmSubChain_H
#define MdsOmSubChain_H

#include "MdsOmChainApp.h"

namespace MdsOmNs {

/**
 * This class is for subscribing to chains.
 * It supports either subscribe(w/ updates) or capture(just initials).
 * There are callbacks for each link as it arrives, and the chain when the last link is done.
 *
 * A chain is composed of links(each of which is a MamaMsg stored in the infrastructure).
 * Each link has elements, which are strings(most commonly used to store symbol names).
 * This link fields have names line LINK_1 or LONGLINK1.
 * There are typically 14 elements in a link msg.
 */
class MDSOMExpDLL MdsOmSubChain
{
public:
	/**
	 * Get a chain with subject(source + symbol) and optional template.
	 * @param om - MdsOm environment.
	 * @param subject - the full subject(source + symbol).
	 * @param templateName - optional template name(may be NULL), found in mama.properties
	 */
	static MdsOmSubChain* factory(MdsOm* om, const char* subject);

	/** 
	 * Get a chain with source + symbol and optional template.
	 * @param om - MdsOm environment.
	 * @param source - the source(e.g., PB-NAMR or IDN_RDF).
	 * @param symbol - the symbol.
	 */
	static MdsOmSubChain* factory(MdsOm* om, const char* source, const char* symbol);

	/**
	 * Destroy the chain.
	 * @return	None.
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void destroy() = 0;

	/** 
	 * Subscribe to a chain.
	 * This returns initials and updates for each link in the chain.
	 * @param cb - interface to receive callbacks on.
	 * @closure closure - provided by app and returned as-is.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void subscribe(MdsOmChainCallback* cb, void* closure) = 0;

	/** 
	 * Capture a chain.
	 * This returns only the initials and the unsubscribes to each link.
	 * @param cb - interface to receive callbacks on.
	 * @closure closure - provided by app and returned as-is.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void capture(MdsOmChainCallback* cb, void* closure) = 0;
	
	/**
	 * Unsubscribe to a chain.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void unsubscribe() = 0;

	/**
	 * Get the number of elements in a chain.
	 * This is a count of the elements in all of the links.
	 * @return the count of elements.
	 */
	virtual size_t getElementCount() const = 0;
	virtual size_t getSubElementCount() const = 0;

	/**
	 * Get the elements in a chain.
	 * This is a list of all of the elements(usually a list of symbol names).
	 * @param l - a list to put the elements in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the elements.
	 * @return the elements.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual MdsOmList<const char*>* getElements(MdsOmList<const char*>* l) const = 0;

	/**
	 * Get the number of links in a chain.
	 * @return the count of links.
	 */
	virtual size_t getLinkCount() const = 0;

	/**
	 * Get the link names in a chain.
	 * The link names are the subject names of each link.
	 * @param l - a list to put the links in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the links.
	 * @return the names.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual MdsOmList<const char*>* getLinkNames(MdsOmList<const char*>* l) const = 0;

	/**
	 * Get the links in a chain.
	 * @param l - a list to put the links in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the links.
	 * @return the links.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual MdsOmList<MdsOmDataLink*>* getLinks(MdsOmList<MdsOmDataLink*>* l) const = 0;

	/**
	 * Clear the chain.
	 * This removes all of the links and elements.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void clearChain() = 0;

	/**
	 * Get the chain name, which is the same as the name of the 1st link.
	 * @return the name.
	 */
	virtual const char* getChainName() const = 0;

	/**
	 * Get the symbol name.
	 * This is the part of the subject after the source.
	 * For example, if the subject is PB-NAMR-DEV.0#AULH6DEL.lonres then the symbol=0#AULH6DEL.lonres.
	 * @return the symbol.
	 */
	virtual const char* getSymbol() const = 0;

	/**
	 * Get the source name.
	 * This is the part of the subject at the beginning.
	 * For example, if the subject is PB-NAMR-DEV.0#AULH6DEL.lonres then the source=PB-NAMR-DEV.
	 * @return the source.
	 */
	virtual const char* getSource() const = 0;

	/**
	 * Get the subject name.
	 * This is the entire name.
	 * For example, the subject is PB-NAMR-DEV.0#AULH6DEL.lonres.
	 * @return the subject.
	 */
	virtual const char* getSubject() const = 0;

	/**
	 * Get the seconds to subscribe to the entire chain.
	 * @return the seconds.
	 */
	virtual time_t getTime() const = 0;

	/**
	 * Returns true if the chain sub is complete, else false.
	 */
	virtual bool getChainDoneSub() const = 0;

protected:
	MdsOmSubChain();
	virtual ~MdsOmSubChain();
};

}

#endif
