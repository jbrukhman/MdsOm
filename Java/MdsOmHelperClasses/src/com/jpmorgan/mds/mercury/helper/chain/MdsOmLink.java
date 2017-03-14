//******************************************************************************
// Implementation module for chain MdsOmLink class
//******************************************************************************

package com.jpmorgan.mds.mercury.helper.chain;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.jpmorgan.mds.mercury.helper.*;
import com.wombat.mama.*;

class MdsOmLink implements MdsOmDataLink, MamaSubscriptionCallback {
	
	private MdsOmChain theChain = null;
	private ChainConfig chainConfig = null;
	private MamaMsg subMsg = null;
	// private boolean beenPublished = false;
	private MdsOmSubscription sub = null;
	private int numUpdates = 0;
	private boolean dirty = false;
	private MdsOmPublisher pub = null;
	private short msgType = MamaMsgType.TYPE_NULL;
	private short status = MamaMsgStatus.STATUS_OK;
	private MdsOmEnv env = null;
	private MamaDictionary dictionary = null;
	private String nextLinkName = "";
	private String prevLinkName = "";

	private String subject = "";
	private String symbol = "";
	private String source = "";
	
	List<MdsOmElement> elementList = new ArrayList<MdsOmElement>();
	Object listLock = new Object();
	
	private short linkType;
	
	public boolean isDirty() { return dirty; }
	public void setDirty(boolean dirty) { this.dirty = dirty; }
	public MdsOmEnv getEnv() { return env; }

	//******************************************************************************
	// Constructor
	//******************************************************************************
	public MdsOmLink()
	{
	}
	
	//******************************************************************************
	// Initialise link
	//******************************************************************************
	public int init(MdsOmChain chain,
						ChainConfig chainConf,
						String source,
						String symbol) 
	{
		theChain = chain;
	
		chainConfig = chainConf;
	
		this.source = source;
		this.symbol = symbol;
		this.subject = this.source + "." + this.symbol;
	
		return 0;
	}
	
	@Override
	public String getSubject() { return subject; }
	
	@Override
	public String getSymbol() { return symbol; }
	
	@Override
	public String getSource() { return source; }
	
	String getNextLinkName() { return nextLinkName; }
	
	//******************************************************************************
	// Destructor - destroy child objects(the elements)
	//******************************************************************************
	public void destroy()
	{
		// Cancel subscripton(if active) and destroy subject
		unsubscribe();
		// The subscriber is now gone, cannot be used again
	
		if (pub != null) {
			// pub.destroy();
			// The publisher is now gone, cannot be used again.
			pub = null;
		}
	
		clearElements();
	}
	
	public MamaMsg getPubMamaMsg() 
	{
		if (pub == null) return null;
		dirty = true;
		return pub.retrieveMamaMsg();
	}
	
	public MamaMsg getSubMamaMsg()
	{
		return subMsg;
	}
	
	// ----------------------------------------------------------------------------
	private MamaMsg setupPub()
	{
		MamaMsg msg = null;
		try {
			if (pub == null) {
				pub = theChain.om.getPublisher(subject, theChain.getPublisherCallback(), theChain.getPublisherClosure());
				msg = pub.getMamaMsg();			// create mama msg
				pub.storeMamaMsg(msg);			// store in publisher for later
				env = pub.getEnv();
				dictionary = env.getDictionary();
			} else {
				msg = pub.retrieveMamaMsg();	// get previously created msg
				msg.clear();
			}
		} catch (Exception ex) {
			// TODO throw ex;
		}
		return msg;
	}
	
	//******************************************************************************
	//
	// Publish the link
	// pref - prefdisplay field for Reuters
	// rdn - rdndisplay field for Reuters
	//
	//******************************************************************************
	public void publish(boolean firstPublish)
	{
		publishOne(firstPublish);
		publishTwo(firstPublish);
	}
	
	public void publishOne(boolean firstPublish)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "Publish: %s", getSubject()));
	
		setupPub();
	}
	
	public void publishTwo(boolean firstPublish)
	{
		MamaMsg msg = pub.retrieveMamaMsg();
		MamaFieldDescriptor field = null;
	
		//--------------------------------------------------------------------------
		if (!dirty && firstPublish == false) {
			Mama.log(MamaLogLevel.FINE, String.format( "Publish: %s link not modified, don't publish", getSubject()));
			return;
		}
	
		List<String> chainFields = chainConfig.getLinkArray();
		int i = 0;
		synchronized (listLock) {
			for (MdsOmElement e : elementList) {
				field = dictionary.getFieldByName(chainFields.get(i++));	
				if (field != null) {
					msg.addString("", field.getFid(), e.getName());
				}
			}
		}
	
		// Fill out the last fields if there are less than 14 elements
		for (; i < chainConfig.getMaxNumLinks(); ++i) {
			field = dictionary.getFieldByName(chainFields.get(i));		
			if (field != null) {
				msg.addString("", field.getFid(), "");
			}
		}
	
		// Set NEXT_LR and PREV_LR fields
		field = dictionary.getFieldByName(chainConfig.getPrevLinkName());		
		if (field != null) {
			msg.addString("", field.getFid(), prevLinkName);
		}
	
		field = dictionary.getFieldByName(chainConfig.getNextLinkName());		
		if (field != null) {
			msg.addString("", field.getFid(), nextLinkName);
		}
	
		// PREF_LINK
		field = dictionary.getFieldByName(chainConfig.getPrefLink());		
		if (field != null) {
			msg.addString("", field.getFid(), prevLinkName);
		}
	
		// REF_COUNT
		field = dictionary.getFieldByName(chainConfig.getRefCount());		
		if (field != null) {
			msg.addU64("", field.getFid(), getElementCount());
		}
	
		short msgType = MamaMsgType.TYPE_UPDATE;
		if (firstPublish) {
			// Send first publish as INITIAL
			msgType = MamaMsgType.TYPE_INITIAL;
		}
		msg.addU8(MamaReservedFields.MsgType.getName(), MamaReservedFields.MsgType.getId(), msgType);
		msg.addU8(MamaReservedFields.MsgStatus.getName(), MamaReservedFields.MsgStatus.getId(), MamaMsgStatus.STATUS_OK);
	
		// Publish it
		// displayMsg(env, source, symbol, msg, 0);
		pub.send(msg);
	
		// beenPublished = true;
		dirty = false;
	}
	
	@SuppressWarnings("unused")
	private void displayMsg(MdsOmEnv env, String source, String symbol, MamaMsg msg, int indent)
	{
		int numFields = msg.getNumFields();
		int bump = 2;
	
		short type = MamaMsgType.TYPE_NULL;
		try { type = MamaMsgType.typeForMsg (msg); } catch (Exception e) { }
	
		int status = MamaMsgStatus.STATUS_OK;
		try { status = MamaMsgStatus.statusForMsg(msg); } catch (Exception e) { }
	
		if (indent == 0) {
			System.out.printf("--- %s.%s payload=%c type=%s status=%s fields=%d ----------------------------\n",
				source, symbol,
				msg.getPayloadType(), MamaMsgType.stringForType(type), MamaMsgStatus.stringForStatus(status), numFields);
		} else {
			System.out.printf("%*s{\n", indent - bump, "");
		}
	
		for (@SuppressWarnings("unchecked")
		Iterator<MamaMsgField> iterator = msg.iterator(); iterator.hasNext();) {
			try {
				MamaMsgField field = iterator.next();
				if (field == null)
					continue;
	
				int fid = field.getFid();
				String name = getFieldName(env, field, fid);
	
				System.out.printf("%*s%*d %-11s%20s ", indent, "", 5, fid, field.getTypeName(), name);
	
				short fieldType = field.getType();
				switch (fieldType) {
				case MamaFieldDescriptor.MSG: {
					MamaMsg tmp = field.getMsg();
					System.out.printf("\n");
					displayMsg(env, source, symbol, tmp, indent + bump);
					break;
				}
				case MamaFieldDescriptor.BOOL: {
					boolean v = field.getBoolean();
					System.out.printf("%d\n", v);
					break;
				}
				case MamaFieldDescriptor.CHAR: {
					char v = field.getChar();
					System.out.printf("%c(%d)\n", v, v); 
					break;
				}
				case MamaFieldDescriptor.I8: {
					byte v = field.getI8();
					System.out.printf("%d(%c)\n", v, v);
					break;
				}
				case MamaFieldDescriptor.U8: {
					short v = field.getU8();
					System.out.printf("%u\n", v);
					break;
				}
				case MamaFieldDescriptor.I16: {
					short v = field.getI16();
					System.out.printf("%d\n", v);
					break;
				}
				case MamaFieldDescriptor.U16: {
					int v = field.getU16();
					System.out.printf("%u\n", v);
					break;
				}
				case MamaFieldDescriptor.I32: {
					int v = field.getI32();
					System.out.printf("%d\n", v);
					break;
				}
				case MamaFieldDescriptor.U32: {
					long v = field.getU32();
					System.out.printf("%u\n", v);
					break;
				}
				case MamaFieldDescriptor.I64: {
					long v = field.getI64();
					System.out.printf("%lld\n", v);
					break;
				}
				case MamaFieldDescriptor.U64: {
					long v = field.getU64();
					System.out.printf("%llu\n", v);
					break;
				}
				case MamaFieldDescriptor.F32: {
					float v = field.getF32();
					System.out.printf("%.4f\n", v);
					break;
				}
				case MamaFieldDescriptor.F64: {
					double v = field.getF64();
					System.out.printf("%.4lf\n", v);
					break;
				}
				case MamaFieldDescriptor.STRING: {
					String v = field.getString();
					System.out.printf("'%s'\n", v);
					break;
				}
				case MamaFieldDescriptor.TIME: {
					MamaDateTime mamaDateTime = field.getDateTime();
					String s = mamaDateTime.getAsString();
					System.out.printf("%s (%d %d)\n", s, mamaDateTime.hasDate(), mamaDateTime.hasTime());
					break;
				}
				case MamaFieldDescriptor.PRICE: {
					MamaPrice mamaPrice = field.getPrice();
					double v = mamaPrice.getValue();
	
					// Get floating point decimal points from mama precision
					MamaPricePrecision prec = mamaPrice.getPrecision();
					int dp = prec.precision2Decimals();
					if (prec == MamaPricePrecision.PRECISION_INT) dp = 0;
	
					System.out.printf("%.*lf(%d %d)\n", dp, v, prec, mamaPrice.getIsValidPrice());
					break;
				} 
				case MamaFieldDescriptor.OPAQUE: {
					byte[] data = field.getOpaque();
					System.out.printf("%d\n", data.length);
					break;
				}
				default:
					String s = field.getString();
					System.out.printf("%s\n", s);
					break; 
				}
			} catch (Exception ex) {
				System.out.printf("%s\n", ex.toString());
			}
		}
	}
		
	private String getFieldName(MdsOmEnv env, MamaMsgField field, int fid)
	{
		String name = null;
		try {
			// Try the dictionary
			name = env.getFieldName(fid);
			if (name == null) {
				// Try the field itself
				name = field.getName();
			}
		} catch (Exception e) {
		}
		if (MdsOmUtil.isBlank(name)) name = "(none)";
		return name;
	}
	
	//******************************************************************************
	//
	// Drop the link
	//
	//******************************************************************************
	public void drop()
	{
		MamaMsg msg = setupPub();
	
		msg.addU8(MamaReservedFields.MsgType.getName(), MamaReservedFields.MsgType.getId(), MamaMsgType.TYPE_DELETE);
		msg.addU8(MamaReservedFields.MsgStatus.getName(), MamaReservedFields.MsgStatus.getId(), MamaMsgStatus.STATUS_OK);
			
		// Send msg
		pub.send(msg);
	}
	
	//******************************************************************************
	// Subscribe to the links subject
	//******************************************************************************
	public void subscribe()
	{
		numUpdates = 0;

		try {
			Mama.log(MamaLogLevel.FINE, String.format( "Chain Subscribe: %s", source, symbol));	
		
			sub = theChain.om.subscribe(source, symbol, this, null, true);
			env = sub.getEnv();
			dictionary = env.getDictionary();
			sub.activate();
		} catch (Exception ex) {
			// TODO throw ex;
		}
	}

	//******************************************************************************
	// Capture to the links subject
	//******************************************************************************
	public void capture()
	{
		numUpdates = 0;
	
		try {
			Mama.log(MamaLogLevel.FINE, String.format( "Capture: %s", source, symbol));	
		
			sub = theChain.om.snap(source, symbol, this, null, true);
			env = sub.getEnv();
			dictionary = env.getDictionary();
			sub.activate();
		} catch (Exception ex) {
			// TODO throw ex;
		}
	}
	
	//******************************************************************************
	// Unsubscribe from the links Teknekron subject
	//******************************************************************************
	public void unsubscribe()
	{
		if (sub != null) {
			sub.destroy();
			sub = null;
		}
	}
	
	public boolean addElement(String name)
	{
		synchronized (listLock) {
			if (isLinkFull())
				 return false;
		
			MdsOmElement e = new MdsOmElement(name);
			elementList.add(e);
		
			return true;
		}
	}
	
	//******************************************************************************
	//
	// Return if the link is full or not
	//
	//******************************************************************************
	public boolean isLinkFull()
	{
		synchronized (listLock) {
			return getElementCount() >= chainConfig.getMaxNumLinks();
		}
	}
	
	//******************************************************************************
	//
	// Get the first element in this link
	//
	//******************************************************************************
	@Override
	public List<String> getElements()
	{
		synchronized (listLock) {
			List<String> l = new ArrayList<String>();
			for (MdsOmElement e : elementList) {
				l.add(e.getName());
			}
			return l;
		}
	}
	
	public void clearElements()
	{
		synchronized (listLock) {
			elementList.clear();
		}
	}
	
	//******************************************************************************
	// Set the name of the next link
	//******************************************************************************
	public void setNextLinkName(String pszNextLinkName)
	{
		nextLinkName = pszNextLinkName;
	}
	
	//******************************************************************************
	// Set name of previous link in chain
	//******************************************************************************
	public void setPrevLinkName(String pszPrevLinkName)
	{
		prevLinkName = pszPrevLinkName;
	}
	
	@Override
	public short getLinkType() { return linkType; }
	
	public void setLinkType(short t) { linkType = t; }
	
	@Override
	public short getStatus() { return status; };
	
	@Override
	public String getChainLinkTypeText(short n) {
		switch (n) {
			case MdsOmDataLink.LINK_TYPE_UNKNOWN:  return "MDS_CHAIN_LINK_TYPE_UNKNOWN";
			case MdsOmDataLink.LINK_TYPE_LINK:     return "MDS_CHAIN_LINK_TYPE_LINK";
			case MdsOmDataLink.LINK_TYPE_LONGLINK: return "MDS_CHAIN_LINK_TYPE_LONGLINK";
		}
		return "Not Recognized???";
	}
	
	@Override
	public int getElementCount() { return elementList.size(); }
	
	public int getMaxElements() { return chainConfig.getMaxNumLinks(); }
	
	@Override
	public int getTotalNumUpdates() { return numUpdates; }
	
	@Override
	public short getSubMamaMsgType() {
		return msgType;
	}

	// methods that mama needs
	@Override
	public void onDestroy(MamaSubscription arg0) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void onCreate(MamaSubscription subscriber)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onCreate: %s", subscriber.getSymbol()));
	}
	
	@Override
	public void onError(MamaSubscription subscription, short mstatus, int platformError, String subject, Exception e)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onError: %s %d", subject, mstatus));
	
		switch (mstatus) {
		case MamaStatus.MAMA_STATUS_OK:					status = MdsOmStatus.OK; break;
		case MamaStatus.MAMA_STATUS_NOT_ENTITLED:		status = MdsOmStatus.CHAIN_NOT_ENTITLED; break;
		case MamaStatus.MAMA_STATUS_NOT_PERMISSIONED:	status = MdsOmStatus.CHAIN_NOT_ENTITLED; break;
		case MamaStatus.MAMA_STATUS_BAD_SYMBOL:			status = MdsOmStatus.CHAIN_BAD_FORMAT; break;
		case MamaStatus.MAMA_STATUS_NOT_FOUND:			status = MdsOmStatus.CHAIN_NOT_FOUND; break;
		case MamaStatus.MAMA_STATUS_TIMEOUT:			status = MdsOmStatus.CHAIN_LINK_TIMEOUT; break;
		// TODO case MamaStatus.MAMA_STATUS_DELETE:				status = MdsOmStatus.CHAIN_LINK_DELETE; break;
		default:										status = MdsOmStatus.CHAIN_ERROR; break;
		}
	
		elementList.clear();
		theChain.errorNextLink(this, status, getSubject());
	}
	
	@Override
	public void onQuality(MamaSubscription subscription, short quality, short cause, Object platformInfo)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onQuality callback"));
	}
	
	@Override
	public void onGap(MamaSubscription subscription)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onGap callback"));
	}
	
	@Override
	public void onRecapRequest(MamaSubscription subscription)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onRecapRequest callback"));
	}
	
	@Override
	public void onMsg(MamaSubscription subscription, MamaMsg msg)
	{
		Mama.log(MamaLogLevel.FINE, String.format( "onMsg: %s", subject));
	
		subMsg = msg;		// only available during the onMsg
	
		numUpdates++;
		msgType = MamaMsgType.typeForMsg(msg);
		int msgStatus = MamaMsgStatus.statusForMsg(msg);
	
		// displayMsg(env, source, symbol, msg, 0);
	
		// we have data
		switch (msgType) {
			case MamaMsgType.TYPE_DELETE:
			case MamaMsgType.TYPE_EXPIRE:
				unsubscribe();
				Mama.log(MamaLogLevel.ERROR, String.format("Symbol deleted or expired. No more subscriptions %s", subject));
				// These actually do not get sent here, they come in onError() cb.
				return;
			default: 
				break;
		}
	
		switch (msgStatus) {
			case MamaMsgStatus.STATUS_BAD_SYMBOL:
			case MamaMsgStatus.STATUS_EXPIRED:
			case MamaMsgStatus.STATUS_TIMEOUT:
			case MamaMsgStatus.STATUS_NOT_PERMISSIONED:
			case MamaMsgStatus.STATUS_NOT_ENTITLED:
			case MamaMsgStatus.STATUS_NOT_FOUND:
				unsubscribe();
				Mama.log(MamaLogLevel.ERROR, String.format("Symbol error. No more subscriptions %s %d", subject, msgStatus));
				return;
			default:
				break;
		}
	
		// we should have a valid subscription
		// look for field names to see what type of links exist
		checkForChain(msg, subscription);
	
		subMsg = null;
	}
	
	private void checkForChain(MamaMsg msg, MamaSubscription subscription)
	{
		MamaFieldDescriptor field = null;
		// short type = MamaMsgType.typeForMsg(msg);
		int refCount = 0;
		MamaString ms = new MamaString();
	
		linkType = MdsOmDataLink.LINK_TYPE_UNKNOWN;
	
		// check for REF_COUNT - if its not there its not a chain
		field = dictionary.getFieldByName("REF_COUNT");
		if (field == null) {
			Mama.log(MamaLogLevel.ERROR, String.format("Dictionary is missing required field 'REF_COUNT'"));
			return;
		}
	
		// REF_COUNT exists - print chain links
		MamaLong ml = new MamaLong();
		boolean b = msg.tryU64(field, ml);
		if (b) {
			refCount = (int) ml.getValue();
			Mama.log(MamaLogLevel.FINE, String.format( "REF_COUNT = %d", refCount));
		} else {
			Mama.log(MamaLogLevel.ERROR, String.format("Message %s is missing required field 'REF_COUNT'", subject));
			return;
		}
	
		nextLinkName = "";
	
		// determine what LINK type to use
		// it could be LINK_x or LONGLINKx
		// see if LINK_1 exists
		field = dictionary.getFieldByName("LINK_1");
		if (field != null) {
			// it exists in dictionary, is it in message ?
			if (msg.tryString(field, ms)) {
				linkType = MdsOmDataLink.LINK_TYPE_LINK;
	
				if ((field = dictionary.getFieldByName("NEXT_LR")) != null) {
					if (msg.tryString(field, ms)) {
						String p = msg.getString(field);
						nextLinkName = p.trim();
					}
				}
	
				if ((field = dictionary.getFieldByName("PREV_LR")) != null) {
					if (msg.tryString(field, ms) == true) {
						String p = msg.getString(field);
						prevLinkName = p.trim();
					}
				}
			}
		}
	
		if (linkType == MdsOmDataLink.LINK_TYPE_UNKNOWN) {
			// try LONGLINK1
			field = dictionary.getFieldByName("LONGLINK1");
			if (field != null) {
				// it exists in dictionary
				if (msg.tryString(field, ms)) {
					linkType = MdsOmDataLink.LINK_TYPE_LONGLINK;
	
					if ((field = dictionary.getFieldByName("LONGNEXTLR")) != null) {
						if (msg.tryString(field, ms)) {
							String p = msg.getString(field);
							nextLinkName = p.trim();
						}
					}
	
					if ((field = dictionary.getFieldByName("LONGPREVLR")) != null) {
						if (msg.tryString(field, ms)) {
							String p = msg.getString(field);
							prevLinkName = p.trim();
						}
					}
				}
			}
		}

		// Does not use recognized link names, can't continue
		if (linkType == MdsOmDataLink.LINK_TYPE_UNKNOWN) return;
	
		if (env.getType() == MdsOm.MdsOmEnvType.MERCURY && nextLinkName.length() > 0) {
			// BAD BAD BAD - for Solace the '.' in a chain next needs to be a '^'
			// NEXT_LR '1#PB.ANY.EMEA_CLIENT.DELTA1.DELTA1'
			boolean foundExchange = false;
			StringBuilder sb = new StringBuilder(nextLinkName);
			for (int i = sb.length() - 1; i >= 0; --i) {
				if (foundExchange) {
					if (sb.charAt(i) == '.') sb.setCharAt(i, '^');
				} else if (sb.charAt(i)  == '.') {
					foundExchange = true;
				}
			}
			nextLinkName = sb.toString();
		}
	
		// Clear elements before getting new ones
		clearElements();
	
		// Get the link data fields
		for (int i = 1; i <= refCount; i++) {
			String linkbuffer = "";
			switch (linkType) {
			case MdsOmDataLink.LINK_TYPE_LINK:
				linkbuffer = "LINK_" + i;
				break;
			case MdsOmDataLink.LINK_TYPE_LONGLINK:
				linkbuffer = "LONGLINK" + i;
				break;
			default:
				break;
			}
	
			field = dictionary.getFieldByName(linkbuffer);
			if (field != null) {
				// it exists in dictionary
				if (msg.tryString(field, ms)) {
					String p = msg.getString(field);
					if (p != null) {
						addElement(p);
					}
				}
			}
		}
	
		theChain.processNextLink(this);
	}
}
