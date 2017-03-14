
package com.jpmorgan.mds.mercury.helper.chain;

import java.util.List;

import com.jpmorgan.mds.mercury.helper.*;

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
public interface MdsOmPubChain
{
		/**
		 * Set the publisher events callback.
		 * @param cb - the callback interface.
		 * @param closure - returned with the callback.
		 */
		public void setPublisherCallback(MdsOmPublisherCallback cb, Object closure);

		/**
		 * Destroy the chain.
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void destroy();

		/**
		 * Publish the chain.
		 * This only publishes links with elements that have changed.
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void publish();

		/**
		 * Publish the entire chain.
		 * It marks all elements as changed before the publish.
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void publishAll();

		/**
		 * Drop the entire chain.
		 * This deletes all of the link msgs from the infrastructure(RMDS or Solace).
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void drop();

		/**
		 * Publish the chain.
		 * This method and PublishTwo are used only if the app wants to add additional fields to the link msgs.
		 * See the MdsOmChain sample app for usage.
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void publishOne();

		/**
		 * Publish the entire chain.
		 * This method and PublishTwo are used only if the app wants to add additional fields to the link msgs.
		 * See the MdsOmChain sample app for usage.
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void publishOneAll();

		/**
		 * Publish the chain.
		 * This method and PublishTwo are used only if the app wants to add additional fields to the link msgs.
		 * See the MdsOmChain sample app for usage.
		 * @throws	MamaStatus, MdsOmStatus
		 */
		public void publishTwo();

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
		 * @return the elements in a list.
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
		 * @return the links.
		 * @param l - a list to put the links in. If NULL then a new list is returned. If not NULL then the param list is cleared first and then loaded with the links.
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public List<MdsOmDataLink> getLinks();

		/**
		 * Add an element to the chain.
		 * @param elementName - the element(usually a symbol name).
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void addElement(String elementName);

		/**
		 * Insert an element to the chain.
		 * @param indx - the index(starts at 1).
		 * @param elementName - the element(usually a symbol name).
		 * @throws MdsOmStatus 
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void insertElement(int indx, String pszElementName) throws MdsOmStatus;

		/**
		 * Remove an element from the chain.
		 * @param indx - the index(starts at 1).
		 * @throws MdsOmStatus 
		 * @throws MamaStatus, MdsOmStatus.
		 */
		public void removeElement(int indx) throws MdsOmStatus;

		/**
		 * Modify an element in the chain.
		 * @param indx - the index(starts at 1).
		 * @param elementName - the element(usually a symbol name).
		 * @throws MdsOmStatus 
		 */
		public void modifyElement(int indx, String pszElementName) throws MdsOmStatus;

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
}
