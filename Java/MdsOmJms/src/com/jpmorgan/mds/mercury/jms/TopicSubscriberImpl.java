package com.jpmorgan.mds.mercury.jms;

import java.util.List;

import javax.jms.InvalidDestinationException;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageListener;
import javax.jms.Topic;
import javax.jms.TopicSubscriber;

import com.jpmorgan.mds.mercury.helper.MdsOm;
import com.jpmorgan.mds.mercury.helper.MdsOmEnv;
import com.jpmorgan.mds.mercury.helper.MdsOmStatus;
import com.jpmorgan.mds.mercury.helper.MdsOmSubscription;
import com.jpmorgan.mds.mercury.helper.chain.*;

import com.wombat.mama.*;

public class TopicSubscriberImpl implements TopicSubscriber, MamaSubscriptionCallback, MdsOmChainCallback {
	private TopicSessionImpl session = null;
	private MdsOmJmsConfig config = null;

	private MdsOmSubChain chain = null;
	private MdsOmSubscription subscr = null;
	private MessageListener listener = null;
	private MdsOm om = null;
	private Topic topic = null;
	private MapMessageImpl mapMsg = null;
	
	protected TopicSubscriberImpl(TopicSessionImpl session, MdsOm om) {
		this.session = session;
		this.om = om;
		config = session.getConn().getConfig();
	}

	protected TopicSessionImpl getSession() {
		return session;
	}

	protected void setSession(TopicSessionImpl session) {
		this.session = session;
		config = session.getConn().getConfig();
	}

	protected MdsOmSubscription getSub() {
		return subscr;
	}

	protected void setSub(MdsOmSubscription sub) {
		this.subscr = sub;
	}

	protected void setTopic(Topic topic) throws JMSException {
		try {
			boolean isChain = false;
			String symbol = topic.getTopicName();
			if (symbol.startsWith("chain.")) {
				int index = symbol.indexOf(".");
				if (index != -1 && index < symbol.length()-1) {
					isChain = true;
					symbol = symbol.substring(++index);
					topic = session.createTopic(symbol);
				}
			}
			
			this.topic = topic;
			if (subscr == null) {
				if (config.isEntitlementsDelegation()) {
					// Using entitlements delegation
					MdsOmEnv env = om.getEnvFromSymbol(topic.getTopicName());
					if (env != null) {
						
					}
				}
				if (isChain) {
					chain = MdsOmChainFactory.factorySub(om, topic.getTopicName());
					chain.subscribeUnderlying(this, this, null);
				} else {
					subscr = om.subscribe(topic.getTopicName(), this,  null);
				}
			}
		} catch (MdsOmStatus e) {
			throw new InvalidDestinationException("Invalid topic: " + topic.getTopicName());
		} catch (JMSException e) {
			throw e;
		}
	}

	@Override
	public Topic getTopic() throws JMSException {
		return topic;
	}

	@Override
	public void close() throws JMSException {
		if (subscr != null) {
			subscr.destroy();
			subscr = null;
		}
		if (chain != null) {
			chain.destroy();
			chain = null;
		}
	}

	@Override
	public MessageListener getMessageListener() throws JMSException {
		return listener;
	}

	@Override
	public String getMessageSelector() throws JMSException {
		return null;
	}

	@Override
	public Message receive() throws JMSException {
		return null;
	}

	@Override
	public Message receive(long arg0) throws JMSException {
		return null;
	}

	@Override
	public Message receiveNoWait() throws JMSException {
		return null;
	}

	@Override
	public void setMessageListener(MessageListener listener) throws JMSException {
		this.listener = listener;
	}

	@Override
	public boolean getNoLocal() throws JMSException {
		return false;
	}
	
	// ---------------------------------------------------------------
	// Callback Functions
	@Override
	public synchronized void mdsOmChainOnSuccess(MdsOmSubChain chain, Object closure)
	{
		try {
			MdsOmEnv env = chain.getEnv();
			List<String> e = chain.getElements();
			for (String v : e) {
				if (v == null) continue;

				MapMessageImpl mm = (MapMessageImpl) session.createMapMessageFromPayload(env);
				MamaMsg mamaMsg = mm.getMamaMsg();
				mm.setMamaMsg(mamaMsg);
				mm.setTopic(session.createTopic(chain.getChainName()));

				// TODO put correct source and symbol into these fields
				mm.setString("MdFeedName", "SOURCE");
				mm.setString("wSymbol", v);
		
				MdsOmJmsMsgType.setMessageType(mm, MdsOmJmsMsgType.STREAM_UPDATE);
				MdsOmJmsMsgStatus.setMamaStatus(mm, MamaStatus.MAMA_STATUS_OK);
	
				if (listener != null) {
					listener.onMessage(mm);
				}
			}
		} catch (JMSException ex) {
			// TODO
			Mama.log(MamaLogLevel.ERROR, "onError: ex=" + ex.getMessage());
		}
	}

	@Override
	public synchronized void mdsOmChainOnLink(MdsOmSubChain chain, MdsOmDataLink link, Object closure)
	{
	}

	@Override
	public synchronized void mdsOmChainOnError(MdsOmSubChain chain, MdsOmDataLink link, short code, Object closure)
	{
		Mama.log(MamaLogLevel.ERROR, String.format("Chain: error with link=%s code=%s", link.getSubject(), MdsOmStatus.stringForStatus(code)));
	}

	// ---------------------------------------------------------------
	// This is called for each market data message
	@Override
	public void onMsg(MamaSubscription sub, MamaMsg msg) {
		// Synchronize with other callbacks and close
		if (subscr == null && chain == null) return;
		try {
			// Cast to MdsOmSubscription
			MdsOmSubscription subscription = (MdsOmSubscription) sub;
			MdsOmEnv env = subscription.getEnv();
			
			if (config.isCopyReceivedMessages()) {
				// TODO check if this is required by JMS spec
				mapMsg = (MapMessageImpl) session.createMapMessageFromPayload(env);
				MamaMsg copy = mapMsg.getMamaMsg();
				copy.copy(msg);
			} else {
				if (mapMsg == null) {
					// Create one reusable map message
					mapMsg = (MapMessageImpl) session.createMapMessageFromPayload(subscription.getEnv());
				}
				mapMsg.setMamaMsg(msg);
			}
			
			mapMsg.setTopic(topic);

			// ------------------------------------------------------------
			// Get the message info
			String symbol = subscription.getSymbol();
			String source = subscription.getSubscSource();
			int status = MamaMsgStatus.statusForMsg(msg);
			short type = MamaMsgType.typeForMsg(msg);
	
			// Mama.log(MamaLogLevel.NORMAL, "JMS.onMsg: symbol=" + symbol + " type=" + type + " status=" + status);
	
			// ------------------------------------------------------------
			// Handle the status
			switch (status) {
			case MamaMsgStatus.STATUS_OK:
				break;
			case MamaMsgStatus.STATUS_BAD_SYMBOL:
			case MamaMsgStatus.STATUS_TIMEOUT:
			case MamaMsgStatus.STATUS_NOT_ENTITLED:
			case MamaMsgStatus.STATUS_NOT_FOUND:
			case MamaMsgStatus.STATUS_NOT_PERMISSIONED:
				if (config.isPrint()) {
					Mama.log(MamaLogLevel.NORMAL,
							"onMsg: " + env.getName() + " " + source + "." + symbol
									+ " " + MamaMsgType.stringForType(type) + " "
									+ MamaMsgStatus.stringForStatus(status));
				}
				// TODO create msg for client
				return;
			case MamaMsgStatus.STATUS_STALE:
			case MamaMsgStatus.STATUS_DUPLICATE:
			case MamaMsgStatus.STATUS_TOPIC_CHANGE:
			default:
				if (config.isPrint()) {
					int nf = msg.getNumFields();
					Mama.log(MamaLogLevel.NORMAL,
							"onMsg: " + env.getName() + " " + source + "." + symbol
									+ " " + MamaMsgType.stringForType(type) + " "
									+ MamaMsgStatus.stringForStatus(status) + " f="
									+ nf);
				}
				// TODO create msg for client
				break;
			}
	
			// ------------------------------------------------------------
			// Handle the message types
			switch (type) {
			case MamaMsgType.TYPE_NOT_PERMISSIONED:
			case MamaMsgType.TYPE_NOT_FOUND:
				if (config.isPrint()) {
					Mama.log(MamaLogLevel.NORMAL,
							"onMsg: " + env.getName() + " " + source + "." + symbol
									+ " " + MamaMsgType.stringForType(type) + " "
									+ MamaMsgStatus.stringForStatus(status));
				}
				// TODO create msg for client
				return;
	
			case MamaMsgType.TYPE_INITIAL:
			case MamaMsgType.TYPE_RECAP:
			case MamaMsgType.TYPE_SNAPSHOT:
			case MamaMsgType.TYPE_BOOK_INITIAL:
			case MamaMsgType.TYPE_UPDATE:
			case MamaMsgType.TYPE_BOOK_UPDATE:
				if (config.isPrint()) {
					Mama.log(MamaLogLevel.NORMAL,
							"onMsg: " + env.getName() + " " + source + "." + symbol
									+ " " + MamaMsgType.stringForType(type) + " "
									+ MamaMsgStatus.stringForStatus(status));
				}
				if (listener != null) {
					if (chain != null) {
						// Update the topic accordingly
						String sym = subscription.getSubscSource() + "." + subscription.getSymbol();
						mapMsg.setTopic(session.createTopic(sym));
					}
					listener.onMessage(mapMsg);
				}
				break;
				
			// TODO check to see what msg types would be handled here ...
			default:
				if (config.isPrint()) {
					Mama.log(MamaLogLevel.NORMAL,
						"onMsg: " + env.getName() + " " + source + "." + symbol
							+ " " + MamaMsgType.stringForType(type) + " "
								+ MamaMsgStatus.stringForStatus(status));
				}
				if (listener != null) {
					listener.onMessage(mapMsg);
				}
				break;
			}
			if (config.isCopyReceivedMessages()) {
				mapMsg = null;			// allow GC on this now if the client does not have a reference
			}
		} catch (Exception ex) {
			Mama.log(MamaLogLevel.ERROR, "onMsg: ex=" + ex.getMessage());
		}
	}

	// Called for each symbol when it becomes active.
	@Override
	public void onCreate(MamaSubscription sub) {
		MdsOmSubscription subscription = (MdsOmSubscription) sub;
		MdsOmEnv env = subscription.getEnv();
		if (config.isPrint()) {
			String symbol = subscription.getSymbol();
			String source = subscription.getSubscSource();
			Mama.log(MamaLogLevel.NORMAL, "Sample.onCreate: " + env.getName()
					+ " " + source + "." + symbol);
		}
	}

	// Called for each symbol when there is an error.
	@Override
	public void onError(MamaSubscription sub, short status, int platformError,
			String symbol, Exception e) {
		MdsOmSubscription subscription = (MdsOmSubscription) sub;
		MdsOmEnv env = subscription.getEnv();
		if (config.isPrint()) {
			String source = subscription.getSubscSource();
			Mama.log(
					MamaLogLevel.WARN,
					"Sample.onError: " + env.getName() + " " + source + " "
							+ symbol + " " + MamaStatus.stringForStatus(status) + " " + 
							+ platformError);
		}
		
		try {
			MapMessageImpl mm = (MapMessageImpl) session.createMapMessageFromPayload(env);
			MamaMsg mamaMsg = mm.getMamaMsg();
			mm.setMamaMsg(mamaMsg);
			mm.setTopic(topic);
			
			MdsOmJmsMsgType.setMessageType(mm, MdsOmJmsMsgType.STATUS);
			MdsOmJmsMsgStatus.setMamaStatus(mm, status);

			if (listener != null) {
				listener.onMessage(mm);
			}
		} catch (JMSException ex) {
			Mama.log(MamaLogLevel.ERROR, "onError: ex=" + ex.getMessage());
		}
	}

	// Called for each subscription when there is a quality issue (e.g., STALE).
	@Override
	public void onQuality(MamaSubscription sub, short quality, short cause,
			Object platformInfo) {
		MdsOmSubscription subscription = (MdsOmSubscription) sub;
		MdsOmEnv env = subscription.getEnv();
		if (config.isPrint()) {
			Mama.log(MamaLogLevel.NORMAL, "Sample.onQuality: " + env.getName()
					+ " " + subscription.getSymbol() + " quality="
					+ MamaQuality.toString(quality) + " cause=" + cause);
		}
		
		try {
			MapMessageImpl mm = (MapMessageImpl) session.createMapMessageFromPayload(env);
			mm.setTopic(topic);
			
			MdsOmJmsMsgType.setMessageType(mm, MdsOmJmsMsgType.STATUS);
			// TODO check quality and cause to set status correctly
			MdsOmJmsMsgStatus.setMessageStatus(mm, MdsOmJmsMsgStatus.STALE);
			
			if (listener != null) {
				listener.onMessage(mm);
			}
		} catch (Exception ex) {
			Mama.log(MamaLogLevel.ERROR, "onQuality: ex=" + ex.getMessage());
		}
	}

	// Called for each subscription when it is destroyed.
	@Override
	public void onDestroy(MamaSubscription sub) {
		MdsOmSubscription subscription = (MdsOmSubscription) sub;
		MdsOmEnv env = subscription.getEnv();
		if (config.isPrint()) {
			String symbol = subscription.getSymbol();
			String source = subscription.getSubscSource();
			Mama.log(MamaLogLevel.NORMAL, "Sample.onDestroy: " + env.getName() + " " + source + "." + symbol);
		}
	}

	// Called for each subscription when there is a gap.
	@Override
	public void onGap(MamaSubscription sub) {
		MdsOmSubscription subscription = (MdsOmSubscription) sub;
		MdsOmEnv env = subscription.getEnv();
		if (config.isPrint()) {
			String symbol = subscription.getSymbol();
			String source = subscription.getSubscSource();
			Mama.log(MamaLogLevel.NORMAL, "Sample.onGap: " + env.getName() + " " + source + "." + symbol);
		}
	}

	// Called for each subscription when there is a recap request.
	@Override
	public void onRecapRequest(MamaSubscription sub) {
		MdsOmSubscription subscription = (MdsOmSubscription) sub;
		MdsOmEnv env = subscription.getEnv();
		if (config.isPrint()) {
			String symbol = subscription.getSymbol();
			String source = subscription.getSubscSource();
			Mama.log(MamaLogLevel.NORMAL, "Sample.onRecapRequest: " + env.getName() + " " + source + "." + symbol);
		}
	}
}
