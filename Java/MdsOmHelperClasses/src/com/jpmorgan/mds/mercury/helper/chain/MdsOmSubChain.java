package com.jpmorgan.mds.mercury.helper.chain;

import java.util.List;

import com.wombat.mama.*;
import com.jpmorgan.mds.mercury.helper.MdsOmStatus;
import com.jpmorgan.mds.mercury.helper.MdsOmEnv;

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
public interface MdsOmSubChain
{
		/**
		 * Destroy the chain.
		 * @return	None.
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void destroy();

		/** 
		 * Subscribe to a chain.
		 * This returns initials and updates for each link in the chain.
		 * @param cb - interface to receive callbacks on.
		 * @param closure - provided by app and returned as-is.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void subscribe(MdsOmChainCallback cb, Object closure) throws MdsOmStatus;

		/** 
		 * Subscribe to a chain.
		 * This returns initials and updates for each link in the chain.
		 * Also each of the elements are subscribed to and returned.
		 * @param cb - interface to receive callbacks on.
		 * @param subCb - interface to receive callbacks on for underlying symbols.
		 * @param - provided by app and returned as-is.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void subscribeUnderlying(MdsOmChainCallback cb, MamaSubscriptionCallback subCb, Object closure) throws MdsOmStatus;

		/** 
		 * Capture a chain.
		 * This returns only the initials and the unsubscribes to each link.
		 * @param cb - interface to receive callbacks on.
		 * @param closure - provided by app and returned as-is.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void capture(MdsOmChainCallback cb, Object closure) throws MdsOmStatus;
		
		/** 
		 * Capture a chain and subscribe to the underlying symbols in the elements.
		 * This returns only the initials and the unsubscribes to each link.
		 * @param cb - interface to receive callbacks on.
		 * @param closure - provided by app and returned as-is.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void captureUnderlying(MdsOmChainCallback cb, MamaSubscriptionCallback subCb, Object closure) throws MdsOmStatus;
		
		/**
		 * Unsubscribe to a chain.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void unsubscribe();

		/**
		 * Get the number of elements in a chain.
		 * This is a count of the elements in all of the links.
		 * @return the count of elements.
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
		 * Get the number of links in a chain.
		 * @return the count of links.
		 */
		public int getLinkCount();

		/**
		 * Get the link names in a chain.
		 * The link names are the subject names of each link.
		 * @param l - a list to put the links in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the links.
		 * @return the names.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public List<String> getLinkNames();

		/**
		 * Get the links in a chain.
		 * @param l - a list to put the links in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the links.
		 * @return the links.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public List<MdsOmDataLink> getLinks();

		/**
		 * Clear the chain.
		 * This removes all of the links and elements.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void clearChain();

		/**
		 * Get the chain name, which is the same as the name of the 1st link.
		 * @return the name.
		 */
		public String getChainName();

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

		/**
		 * Get the seconds to subscribe to the entire chain.
		 * @return the seconds.
		 */
		public long getTime();

		/**
		 * Returns true if the chain sub is complete, else false.
		 */
		public boolean getChainDoneSub();

		public MdsOmEnv getEnv();
}
