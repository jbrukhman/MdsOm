
package com.jpmorgan.mds.mercury.helper.chain;

import java.util.List;

import com.wombat.mama.*;

public interface MdsOmDataLink
{
	 /** Not yet determined. */
	public static final short LINK_TYPE_UNKNOWN = 0;
	/** Uses LINK field names. */
	public static final short LINK_TYPE_LINK = 1;
	/** Uses LONGLINK field names. */
	public static final short LINK_TYPE_LONGLINK = 2;
	
	/**
	 * Get the MamaMsg for a subscriber.
	 * Only valid during a link callback.
	 * @return ptr to the MamaMsg.
	 */
	public MamaMsg getSubMamaMsg();

	/**
	 * Get the msg type for the most recent MamaMsg.
	 * @return the mama msg type.
	 */
	public short getSubMamaMsgType();

	/**
	 * Get the MamaMsg for a publisher.
	 * Only valid if using the PublishOne() and PublishTwo() approach to publishing.
	 * NOTE: this sets the link as dirty.
	 */
	public MamaMsg getPubMamaMsg();

 	/** Get the number of elements in this link.
	 * @return The number of elements.
	 */
	public int getElementCount();

	/**
	 * Get the elements in a chain.
	 * This is a list of all of the elements(usually a list of symbol names).
	 * @param l - a list to put the elements in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the elements.
	 * @return the elements.
	 * @throws MamaStatus, MdsOmStatus.
	 */
	public List<String> getElements();

	/**
	 * Get the symbol name.
	 * This is the part of the subject after the source.
	 * For example, if the subject is PB-NAMR-DEV.0#AULH6DEL.lonres then the symbol=0#AULH6DEL.lonres.
	 * @return the symbol.
	 */
	public String getSymbol();

	/**
	 * Get the source name.
	 * This is the part of the subject at the beginning.
	 * For example, if the subject is PB-NAMR-DEV.0#AULH6DEL.lonres then the source=PB-NAMR-DEV.
	 * @return the source.
	 */
	public String getSource();

	/**
	 * Get the subject name.
	 * This is the entire name.
	 * For example, the subject is PB-NAMR-DEV.0#AULH6DEL.lonres.
	 * @return the subject.
	 */
	public String getSubject();

	/** Get the link type for this chain */
	public short getLinkType();

	/** Get the text version of the MDSChainLinkType enum. */
	public String getChainLinkTypeText(short t);

	public int getTotalNumUpdates();

	public short getStatus();
	
}

