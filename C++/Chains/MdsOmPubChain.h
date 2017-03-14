#ifndef MdsOmPubChain_H
#define MdsOmPubChain_H

#include "MdsOmChainApp.h"

namespace MdsOmNs {

/**
 * This class is for publishing chains.
 *
 * A chain is composed of links(each of which is a MamaMsg stored in the infrastructure).
 * Each link has elements, which are strings(most commonly used to store symbol names).
 * This link fields have names line LINK_1 or LONGLINK1.
 * There are typically 14 elements in a link msg.

 * A template name may be used. The template is located in the mama.properties file.
 * If there is no template then the link names LINK! ... LINK14 are used.
 */
class MDSOMExpDLL MdsOmPubChain
{
public:
	/**
	 * Get a chain with subject(source + symbol) and optional template.
	 * @param om - MdsOm environment.
	 * @param subject - the full subject(source + symbol).
	 * @param templateName - optional template name(may be NULL), found in mama.properties
	 */
	static MdsOmPubChain* factory(MdsOm* om, const char* subject, const char* templateName);

	/** 
	 * Get a chain with source + symbol and optional template.
	 * @param om - MdsOm environment.
	 * @param source - the source(e.g., PB-NAMR or IDN_RDF).
	 * @param symbol - the symbol.
	 * @param templateName - optional template name(may be NULL), found in mama.properties
	 */
	static MdsOmPubChain* factory(MdsOm* om, const char* source, const char* symbol, const char* templateName);

	/**
	 * Set the publisher events callback.
	 * @param cb - the callback interface.
	 * @param closure - returned with the callback.
	 */
	virtual void setPublisherCallback(MdsOmPublisherCallback* cb, void* closure) = 0;

	/**
	 * Destroy the chain.
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void destroy() = 0;

	/**
	 * Publish the chain.
	 * This only publishes links with elements that have changed.
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void publish() = 0;

	/**
	 * Publish the entire chain.
	 * It marks all elements as changed before the publish.
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void publishAll() = 0;

	/**
	 * Drop the entire chain.
	 * This deletes all of the link msgs from the infrastructure(RMDS or Solace).
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void drop() = 0;

	/**
	 * Publish the chain.
	 * This method and PublishTwo are used only if the app wants to add additional fields to the link msgs.
	 * See the MdsOmChain sample app for usage.
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void publishOne() = 0;

	/**
	 * Publish the entire chain.
	 * This method and PublishTwo are used only if the app wants to add additional fields to the link msgs.
	 * See the MdsOmChain sample app for usage.
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void publishOneAll() = 0;

	/**
	 * Publish the chain.
	 * This method and PublishTwo are used only if the app wants to add additional fields to the link msgs.
	 * See the MdsOmChain sample app for usage.
	 * @throws	MamaStatus, MdsOmStatus
	 */
	virtual void publishTwo() = 0;

	/**
	 * Get the number of elements in a chain.
	 * This is a count of the elements in all of the links.
	 * @return the count of elements.
	 */
	virtual size_t getElementCount() const = 0;
	virtual size_t getPubElementCount() const = 0;

	/**
	 * Get the elements in a chain.
	 * This is a list of all of the elements(usually a list of symbol names).
	 * @param l - a list to put the elements in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the elements.
	 * @return the elements in a list.
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
	 * @return the links.
	 * @param l - a list to put the links in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the links.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual MdsOmList<MdsOmDataLink*>* getLinks(MdsOmList<MdsOmDataLink*>* l) const = 0;

	/**
	 * Add an element to the chain.
	 * @param elementName - the element(usually a symbol name).
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void addElement(const char* elementName) = 0;

	/**
	 * Insert an element to the chain.
	 * @param indx - the index(starts at 1).
	 * @param elementName - the element(usually a symbol name).
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void insertElement(int indx, const char *pszElementName) = 0;

	/**
	 * Remove an element from the chain.
	 * @param indx - the index(starts at 1).
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void removeElement(int indx) = 0;

	/**
	 * Modify an element in the chain.
	 * @param indx - the index(starts at 1).
	 * @param elementName - the element(usually a symbol name).
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual void modifyElement(int indx, const char *pszElementName) = 0;

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

protected:
	MdsOmPubChain();
	virtual ~MdsOmPubChain();
};

}

#endif
