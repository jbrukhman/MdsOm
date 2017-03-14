
#ifndef MdsOmDataLink_H
#define MdsOmDataLink_H

#include "MdsOmChainApp.h"

namespace MdsOmNs {

// -------------------------------------------------
/**
 * Class used to access Link level data.
 */
class MDSOMExpDLL MdsOmDataLink
{
public:

	MdsOmDataLink();
	virtual ~MdsOmDataLink();

	/**
	 * Get the MamaMsg for a subscriber.
	 * Only valid during a link callback.
	 * @return ptr to the MamaMsg.
	 */
	virtual Wombat::MamaMsg* getSubMamaMsg() const = 0;

	/**
	 * Get the msg type for the most recent MamaMsg.
	 * @return the mama msg type.
	 */
	virtual mamaMsgType getSubMamaMsgType() const = 0;

	/**
	 * Get the MamaMsg for a publisher.
	 * Only valid if using the PublishOne() and PublishTwo() approach to publishing.
	 * NOTE: this sets the link as dirty.
	 */
	virtual Wombat::MamaMsg* getPubMamaMsg() const = 0;

 	/** Get the number of elements in this link.
	 * @return The number of elements.
	 */
	virtual size_t getElementCount() const = 0;

	/**
	 * Get the elements in a chain.
	 * This is a list of all of the elements(usually a list of symbol names).
	 * @param l - a list to put the elements in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the elements.
	 * @return the elements.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	virtual MdsOmList<const char*>* getElements(MdsOmList<const char*>* l) = 0;

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
	 * Returns the NEXT_LR field, used only for debugging and info.
	 */
	virtual const char* getNextLinkName() const = 0;

	/** Get the link type for this chain */
	virtual MdsOmChainLinkType getLinkType() const = 0;

	/** Get the text version of the MDSChainLinkType enum. */
	virtual const char* getChainLinkTypeText(MdsOmChainLinkType t) const = 0;

	virtual size_t getTotalNumUpdates() const = 0;

	virtual MdsOmStatusCode getStatus() const = 0;
};

}

#endif
