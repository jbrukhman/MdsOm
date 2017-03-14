//
// MdsOmChain
// This is the chain helper class for OpenMAMA
//

package com.jpmorgan.mds.mercury.helper.chain;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

import com.jpmorgan.mds.mercury.helper.*;
import com.wombat.mama.*;

public class MdsOmChain implements MdsOmSubChain, MdsOmPubChain, MamaSubscriptionCallback
{
	protected MdsOm om = null;
	private MdsOmEnv env = null;
	@SuppressWarnings("unused")
	private MamaDictionary dictionary = null;
	private String templateName = "";
	private boolean subUnder = false;
	
	private ChainConfig chainConfig = null;
	
	private List<MdsOmSubscription> subs = new ArrayList<MdsOmSubscription>();
	private List<MdsOmLink> linkList = new ArrayList<MdsOmLink>();
	private List<MdsOmElement> elementList = new ArrayList<MdsOmElement>();
	private Object listLock = new Object();
	
	private Object closure = null;
	private MamaSubscriptionCallback subCb = null;
	private MdsOmChainCallback cb = null;
	private MdsOmPublisherCallback pubCb = null;
	private Object pubClosure = null;
	
	private String source = "";
	private String symbol = "";
	private String subject = "";
	
	short subType;
	boolean firstPublish = true;
	boolean chainDoneSub = false;
	
	long startTime = 0;
	long elapsedTime = 0;
	
	@Override
	public long getTime() { return elapsedTime; }

	@Override
	public MdsOmEnv getEnv() { return env; }

	MdsOmChain(MdsOm om, String source, String symbol, String templateName) throws MdsOmStatus
	{
		this.om = om;
		if (MdsOmUtil.isNotBlank(templateName)) this.templateName = templateName;
		if (MdsOmUtil.isNotBlank(source)) this.source = source;
		if (MdsOmUtil.isNotBlank(symbol)) this.symbol = symbol;
		this.source = source;
		this.symbol = symbol;
		try {
			init();
		} catch (MdsOmStatus e) {
			throw e;
		}
	}

	MdsOmChain(MdsOm om, String subject, String templateName) throws MdsOmStatus
	{
		this.om = om;
		if (MdsOmUtil.isNotBlank(templateName)) this.templateName = templateName;
		if (MdsOmUtil.isNotBlank(subject)) {
			Pair<String, String> tokens = MdsOmUtil.getSourceAndSymbol(subject);
			this.source = tokens.getX();
			this.symbol = tokens.getY();
		}
		try {
			init();
		} catch (MdsOmStatus e) {
			throw e;
		}
	}

	@Override
	public void destroy()
	{
		synchronized (listLock) {
			// Clear any underlying subscriptions
			for (MdsOmSubscription sub : subs) {
				sub.destroy();
			}
			subs.clear();

			// Delete the links
			linkList.clear();
		
			elementList.clear();
		}
	}

	/**
	 * Drop the links in the infrastructure.
	 */
	@Override
	public void drop()
	{
		synchronized (listLock) {
			if (linkList.size() == 0) {
				// Add a link to delete the first link.
				// This may not delete all of the links for the chain.
				addLink(source, symbol);
			}
		
			for (MdsOmLink link : linkList) {
				link.drop();
			}
		
			clearChain();
		}
	}

	private void init() throws MdsOmStatus
	{
		// Validate name
		if (symbol.isEmpty()) {
			Mama.log(MamaLogLevel.ERROR, String.format("Init: no symbol provided '%s'", symbol));
			throw new MdsOmStatus(MdsOmStatus.CHAIN_BAD_FORMAT);
		}

		char c = symbol.charAt(0);
		if (!Character.isDigit(c)) {
			Mama.log(MamaLogLevel.ERROR, String.format("Init: missing digit in chain name '%s'", symbol));
			throw new MdsOmStatus(MdsOmStatus.CHAIN_BAD_FORMAT);
		}
	
		if (!symbol.contains("#")) {
			Mama.log(MamaLogLevel.ERROR, String.format("Init: missing '#' in chain name '%s'", symbol));
			throw new MdsOmStatus(MdsOmStatus.CHAIN_BAD_FORMAT);
		}
	
		subject = source + "." + symbol;
	
		getConfig();
	}

	// This is called by the app when it wants publisher feedback
	@Override
	public void setPublisherCallback(MdsOmPublisherCallback cb, Object closure) {	pubCb = cb; pubClosure = closure; }

	MdsOmPublisherCallback getPublisherCallback() { return pubCb; }
	
	Object getPublisherClosure() { return pubClosure; }
	
	@Override
	public String getChainName() { return subject; }
	
	@Override
	public String getSubject() { return subject; }
	
	@Override
	public String getSymbol()	{ return symbol; }
	
	@Override
	public String getSource()	{ return source; }
	
	@Override
	public boolean getChainDoneSub() { return chainDoneSub; }

	void getConfig()
	{
		chainConfig = new ChainConfig(templateName);
		chainConfig.Init();
	}
	
	@Override
	public void subscribeUnderlying(MdsOmChainCallback cb, MamaSubscriptionCallback subCb, Object closure) throws MdsOmStatus
	{
		subUnder = true;
		this.subCb = subCb;
		subscribe(cb, closure);
	}

	@Override
	public void subscribe(MdsOmChainCallback cb, Object closure) throws MdsOmStatus
	{
		this.cb = cb;
		this.closure = closure;
	
		subType = MamaServiceLevel.REAL_TIME;
		chainDoneSub = false;
	
		Mama.log(MamaLogLevel.FINE, String.format("Chain::Subscribe: %s.%s", source, symbol));
	
		if (om == null || MdsOmUtil.isBlank(source) || MdsOmUtil.isBlank(symbol)) {
			Mama.log(MamaLogLevel.ERROR, String.format("Subscribe: NULL parms om=%p source='%s' symbol='%s'", om, source, symbol));
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
	
		startTime = System.currentTimeMillis() / 1000;
		subscribeLink(source, symbol);
	}
	
	// Called on the init and for each subsequent link
	void subscribeLink(String source, String symbol)
	{
		MdsOmLink link = addLink(source, symbol);
	
		switch (subType) {
			case MamaServiceLevel.SNAPSHOT:
				link.capture();
				break;
			case MamaServiceLevel.REAL_TIME:
			default:
				link.subscribe();
				break;
		}

		// Get this for the first link, should be the same for all, right?
		if (env == null) env = link.getEnv();
	}
	
	MdsOmLink addLink(String source, String symbol)
	{
		MdsOmLink link = new MdsOmLink();
		link.init(this, chainConfig, source, symbol);
		synchronized (listLock) {
			linkList.add(link);
			return link;
		}
	}
	
	@Override
	public void unsubscribe()
	{
		// Clear any underlying subscriptions
		for (MdsOmSubscription sub : subs) {
			sub.destroy();
		}
		subs.clear();

		// This removes all links and elements
		clearChain();
	}
	
	@Override
	public void captureUnderlying(MdsOmChainCallback cb, MamaSubscriptionCallback subCb, Object closure) throws MdsOmStatus
	{
		subUnder = true;
		this.subCb = subCb;
		capture(cb, closure);
	}

	@Override
	public void capture(MdsOmChainCallback cb, Object closure) throws MdsOmStatus
	{
		this. cb = cb;
		this.closure = closure;
	
		subType = MamaServiceLevel.SNAPSHOT;
		chainDoneSub = false;
	
		Mama.log(MamaLogLevel.FINE, String.format("CaptureChainsSubscribe: %s.%s", source, symbol));
	
		if (om == null || MdsOmUtil.isBlank(source) || MdsOmUtil.isBlank(symbol)) {
			Mama.log(MamaLogLevel.ERROR, String.format("Subscribe: NULL parms om=%p source='%s' symbol='%s'", om, source, symbol));
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
	
		// Clear the chain before capture, since they can call capture many times on a single chain
		clearChain();
		
		// Subscribe to first link in chain
		startTime = System.currentTimeMillis() / 1000;
		subscribeLink(source, symbol);
	}
	
	// This is called by the link when it finishes processing its msg
	void processNextLink(MdsOmLink link)
	{
		String nextLinkName = link.getNextLinkName();
	
		Mama.log(MamaLogLevel.FINE, String.format("ProcessNextLink: %s '%s'", source, nextLinkName));
	
		if (nextLinkName.isEmpty()) {
			// All done with chain, call the app
			long end = System.currentTimeMillis() / 1000;
			elapsedTime = end - startTime;
	
			// Delete any remaining links
			removeLinksAfter(link);
	
			if (cb != null) cb.mdsOmChainOnLink(this, link, closure);

			// Only call chain callback when done and do it only once
			// Later link updates will send link cb, but not chain cb
			if (cb != null && !chainDoneSub) cb.mdsOmChainOnSuccess(this, closure);
	
			chainDoneSub = true;

			if (subUnder) {
				// Subscribe to underlying symbols now
				List<String> e = getElements();
				for (String s : e) {
					try {
						String symbol = source + "." + s;
						MdsOmSubscription sub = om.subscribe(symbol, this, null);
						subs.add(sub);
					} catch (MdsOmStatus ex) {
						// TODO
						System.err.println("ERROR: " + ex.getMessage());
					}
				}
			}
		} else {
			// Subscribe to next link
			boolean isLast = isLastLink(link);
			Mama.log(MamaLogLevel.FINE, String.format("ProcessNextLink: %s '%s' last=%b", source, nextLinkName, isLast));
			if (cb != null) cb.mdsOmChainOnLink(this, link, closure);
			if (isLast) {
				// Last link in our list, and we have a next name, so keep subscribing
				subscribeLink(source, nextLinkName);
			}
		}
	}
	
	// This is called by the link when it finishes processing its msg
	void errorNextLink(MdsOmLink link, short code, String subject)
	{
		long end = System.currentTimeMillis() / 1000;
		elapsedTime = end - startTime;
	
		// Error with the chain, call the app
		if (cb != null) cb.mdsOmChainOnError(this, link, code, closure);
		chainDoneSub = true;
	}
	
	boolean isLastLink(MdsOmLink link)
	{
		synchronized (listLock) {
			if (linkList.size() == 0) return false;
			return link == linkList.get(linkList.size() - 1);
		}
	}
	
	// Used to remove links after a link with no next field
	void removeLinksAfter(MdsOmLink after)
	{
		synchronized (listLock) {
			boolean removing = false;
			ListIterator<MdsOmLink> iter = linkList.listIterator();
			while (iter.hasNext()) {
				MdsOmLink link = iter.next();
				if (removing) {
			        iter.remove();
			    }
				if (link == after) {
					removing = true;
				}
			}
		}
	}
	
	@Override
	public int getLinkCount()
	{
		synchronized (listLock) {
			return linkList.size();
		}
	}
	
	@Override
	public int getElementCount()
	{
		synchronized (listLock) {
			int count = 0;
			for (MdsOmLink link : linkList) {
				count += link.getElementCount();
			}
			return count;
		}
	}
	
	@Override
	public List<String> getElements()
	{
		synchronized (listLock) {
			// This will return all of the elements of elementList
			List<String> l = new ArrayList<String>();
			for (MdsOmLink link : linkList) {
				for (MdsOmElement e : link.elementList) {
					l.add(e.getName());
				}
			}
			return l;
		}
	}
	
	/** Get all of the link names(subjects)
	 */
	@Override
	public List<String> getLinkNames()
	{
		synchronized (listLock) {
			// This will return all of the linknames
			List<String> l = new ArrayList<String>();
			for (MdsOmLink link : linkList) {
				l.add(link.getSubject());
			}
			return l;
		}
	}
	
	@Override
	public List<MdsOmDataLink> getLinks()
	{
		synchronized (listLock) {
			// This will return all of the linknames
			List<MdsOmDataLink> l = new ArrayList<MdsOmDataLink>();
			for (MdsOmDataLink link : linkList) {
				l.add(link);
			}
			return l;
		}
	}
	
	//******************************************************************************
	// Add an element to the chain at the end
	//******************************************************************************
	@Override
	public void addElement(String name)
	{
		synchronized (listLock) {
			// Insert at end of elementList
			if (name == null) name = "";	
			MdsOmElement e = new MdsOmElement(name);
			elementList.add(e);
			e.setDirty(true);
		}
	}
	
	//******************************************************************************
	// Delete an element from the chain at position indx(1-based)
	//******************************************************************************
	@Override
	public void removeElement(int indx) throws MdsOmStatus
	{
		synchronized (listLock) {
			// The index is 1-based
			if (indx <= 0 || elementList.size() == 0 || elementList.size() < indx) throw new MdsOmStatus(MdsOmStatus.CHAIN_BAD_INDEX);
			indx--;
		
			elementList.remove(indx);
		
			// Set the rest as dirty
			for (; indx < elementList.size(); ++indx) {
				MdsOmElement e = elementList.get(indx);
				e.setDirty(true);
			}
		}
	}
	
	//******************************************************************************
	// Update an element in the chain at position indx(1-based)
	//******************************************************************************
	@Override
	public void insertElement(int indx, String name) throws MdsOmStatus
	{
		synchronized (listLock) {
			// The index is 1-based
			if (indx <= 0 || elementList.size() == 0 || elementList.size() < indx) throw new MdsOmStatus(MdsOmStatus.CHAIN_BAD_INDEX);
			indx--;
		
			if (name == null) name = "";
		
			MdsOmElement e = new MdsOmElement(name);
			
			elementList.add(indx, e);
		
			// Set the rest as dirty
			for (; indx < elementList.size(); ++indx) {
				e = elementList.get(indx);
				e.setDirty(true);
			}
		}
	}
	
	//******************************************************************************
	// Update an element in the chain at position indx(1-based)
	//******************************************************************************
	@Override
	public void modifyElement(int indx, String name) throws MdsOmStatus
	{
		synchronized (listLock) {
			// The index is 1-based
			if (indx <= 0 || elementList.size() == 0 || elementList.size() < indx) throw new MdsOmStatus(MdsOmStatus.CHAIN_BAD_INDEX);
			indx--;
			
			MdsOmElement e = elementList.get(indx);
			e.setName(name);
		
			// Set the rest as dirty
			for (; indx < elementList.size(); ++indx) {
				e = elementList.get(indx);
				e.setDirty(true);
			}
		}
	}
	
	@Override
	public void clearChain()
	{
		clearLinks();
		clearElements();
	}
	
	void clearElements()
	{
		synchronized (listLock) {
			elementList.clear();
		}
	}
	
	void clearLinks()
	{
		synchronized (listLock) {
			linkList.clear();
		}
	}
	
	/** Used when creating links before a publish.
	 * The elements are kept in elementList, and then the links are
	 * created from that just before a publish.
	 */
	void addElement2(MdsOmElement e)
	{
		synchronized (listLock) {
			MdsOmLink link;
			if (linkList.size() == 0) {
				// No links at all, add first one with chain subject
				link = addLink(source, symbol);
			} else {
				// Get the end of the list
				link = linkList.get(linkList.size() - 1);
			}
		
			boolean ret = link.addElement(e.getName());
			if (ret) {
				if (e.isDirty()) link.setDirty(true);
			} else {
				// Last link was full, add another one
				// Get next link name
				MdsOmChainName chainName = new MdsOmChainName();
				chainName.init(source, link.getSymbol(), chainConfig);
				String next = chainName.getNextLinkSubject();
		
				MdsOmLink newLink = addLink(source, next);
				ret = newLink.addElement(e.getName());				// this sets the link dirty if the element is dirty
				if (e.isDirty()) newLink.setDirty(true);
				if (e.isDirty()) link.setDirty(true);				// set prev link dirty to publish it, since it contains the link to this new link
			}
		}
	}
	
	/**
	 * Publish a chain.
	 * This will create the links based on the elements in elementList.
	 */
	@Override
	public void publish()
	{
		publishOne();
		publishTwo();
	}
	
	/**
	 * Publish ALL of a chain to refresh.
	 * This will create the links based on the elements in elementList.
	 */
	@Override
	public void publishAll()
	{
		synchronized (listLock) {
			for (MdsOmElement e : elementList) {
				e.setDirty(true);
			}
			publish();
		}
	}
	
	@Override
	public void publishOneAll()
	{
		synchronized (listLock) {
			for (MdsOmElement e : elementList) {
				e.setDirty(true);
			}
			publishOne();
		}
	}
	
	/** This is only used by an app that wants to add extra fields.
	 * To do that call PublishOne(), the get the MamaMsg and add fields, and then call PublishTwo().
	 */
	@Override
	public void publishOne()
	{
		synchronized (listLock) {
			clearLinks();				// remove previous links
		
			// Add the elements as links
			for (MdsOmElement e : elementList) {
				addElement2(e);
			}
		
			if (elementList.size() == 0) {
				// Empty list, publish a single link with empty elements
				MdsOmLink link = addLink(source, symbol);
				link.setDirty(true);
			}
		
			// Set prev/next names
			String prevName = "";
			MdsOmLink prev = null;
			for (MdsOmLink link : linkList) {
				link.setPrevLinkName(prevName);
				link.setNextLinkName("");				// to get last one
				prevName = link.getSymbol();
				if (prev != null) prev.setNextLinkName(prevName);
				prev = link;
			}
		
			// Mark all elements as published
			for (MdsOmElement e : elementList) {
				e.setDirty(false);
			}
		
			// Now publish One
			for (MdsOmLink link : linkList) {
				link.publishOne(firstPublish);
			}
		}
	}
	
	@Override
	public void publishTwo()
	{
		synchronized (listLock) {
			// Now publish.
			for (MdsOmLink link : linkList) {
				if (link.isDirty() || firstPublish) {
					link.publishTwo(firstPublish);
				}
			}
		
			firstPublish = false;
		}
	}

	// methods that mama needs
	@Override
	public void onDestroy(MamaSubscription subscription) {
		if (subCb != null) subCb.onDestroy(subscription);
	}

	@Override
	public void onCreate(MamaSubscription subscription)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onCreate: %s", subscription.getSymbol()));
		if (subCb != null) subCb.onCreate(subscription);
	}
	
	@Override
	public void onError(MamaSubscription subscription, short mstatus, int platformError, String subject, Exception e)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onError: %s %d", subject, mstatus));
		if (subCb != null) subCb.onError(subscription, mstatus, platformError, subject, e);
	}
	
	@Override
	public void onQuality(MamaSubscription subscription, short quality, short cause, Object platformInfo)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onQuality callback"));
		if (subCb != null) subCb.onQuality(subscription, quality, cause, platformInfo);
	}
	
	@Override
	public void onGap(MamaSubscription subscription)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onGap callback"));
		if (subCb != null) subCb.onGap(subscription);
	}
	
	@Override
	public void onRecapRequest(MamaSubscription subscription)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onRecapRequest callback"));
		if (subCb != null) subCb.onRecapRequest(subscription);
	}
	
	@Override
	public void onMsg(MamaSubscription subscription, MamaMsg msg)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onMsg: %s", subject));
		if (subCb != null) subCb.onMsg(subscription, msg);
	}
}

