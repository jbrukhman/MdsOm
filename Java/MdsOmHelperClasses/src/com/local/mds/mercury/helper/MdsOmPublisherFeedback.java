package com.jpmorgan.mds.mercury.helper;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import com.wombat.mama.Mama;
import com.wombat.mama.MamaBasicSubscription;
import com.wombat.mama.MamaBasicSubscriptionCallback;
import com.wombat.mama.MamaLogLevel;
import com.wombat.mama.MamaMsg;
import com.wombat.mama.MamaTransport;
import com.wombat.mama.MamaString;
import com.wombat.mama.MamaInteger;


/**
 * This is used to manage the publisher feedback for Solace and Tick42.
 * Currently they do this differntly; this will change when OM publisher events is released.
 * @author u045594
 *
 */
public class MdsOmPublisherFeedback implements MamaBasicSubscriptionCallback {
	
	private Set<String> transportSet = new HashSet<String>();
	private Map<String, Set<MdsOmPublisher>> feedbackMap = new HashMap<String, Set<MdsOmPublisher>>();
	
	public MdsOmPublisherFeedback()
	{
	}

	public void addPublisher(MdsOmPublisher p)
	{
		MdsOmEnv env = p.getEnv();

		// Check the config the turn on/off the Solace async publisher feedback
		String buf = "mama." + env.getBridgeName() + ".transport." + p.getMamaTransport().getName() + ".publisherFeedback";
		String feedStr = Mama.getProperty(buf);
		Mama.log(MamaLogLevel.FINE, "MdsOmPublisherFeedback::addPublisher: " + buf + " " + feedStr);
		if (feedStr == null || feedStr.compareToIgnoreCase("true") != 0) return;

		synchronized (this) {
			try {
				MamaTransport t = p.getMamaTransport();
				boolean b = transportSet.contains(t.getName());
				if (!b) {
					// Don't have existing feedback subscription for this transport, so add one
					// There is only 1 feedback sub per transport
					transportSet.add(t.getName());
	
					Mama.log(MamaLogLevel.NORMAL, "MdsOmPublisherFeedback::addPublisher: " + t.getName() + " " + p.getSubject());

					MamaBasicSubscription feedback = new MamaBasicSubscription();
					feedback.createBasicSubscription(this, t, env.getQueue(), "_SOLACE.TOPIC_TRANSPORT_CB_EX", null);
				}
	
				// Add the publisher to the list of listeners
				Set<MdsOmPublisher> pl = feedbackMap.get(p.getSubject());
				if (pl == null) {
					pl = new HashSet<MdsOmPublisher>();
					feedbackMap.put(p.getSubject(), pl);
				}
				// Use set here to get uniqueness
				pl.add(p);
	
			} catch (Exception status) {
				Mama.log(MamaLogLevel.ERROR, "MdsOmPublisherFeedback::addPublisher: error with solace feedback " + p.getSubject() + " " + status.toString());
			}
		}
	}

	public synchronized void removePublisher(MdsOmPublisher p)
	{
		try {
			Set<MdsOmPublisher> pl = feedbackMap.get(p.getSubject());
			if (pl != null) {
				pl.remove(p);
			}
		} catch (Exception status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisherFeedback::solacePublishFail: error with solace feedback " + p.getSubject() + " " + status.toString());
		}
	}

	// SOLACE feedback subscriber
	@Override
	public void onCreate (MamaBasicSubscription  subscription)
	{
	}

	@Override
	public void onMsg (
	    MamaBasicSubscription  subscription,
	    MamaMsg                msg)
	{
		solacePublishFail(msg);
	}

	@Override
	public void onDestroy(MamaBasicSubscription subscription) {
		String topic = subscription.getSymbol();
		Mama.log(MamaLogLevel.ERROR, "MdsOmPublisherFeedback::onDestroy: " + topic);
	}

	@Override
	public void onError(MamaBasicSubscription subscription, short arg1, int arg2, String arg3) {
		String topic = subscription.getSymbol();
		Mama.log(MamaLogLevel.ERROR, "MdsOmPublisherFeedback::onDestroy: " + topic);		
	}
	
	// SOLACE & SUB FEEDBACK
	// This is for temporary Solace publish feedback.
	// Will be replaced by OM solution.
	// TODO lock is held for along time, but this is only for publish errors
	public synchronized void solacePublishFail(MamaMsg msg)
	{
		try {
			// These are the fields that Solace sends
			MamaInteger eventType = new MamaInteger();
			MamaInteger mamaStatus = new MamaInteger();
			MamaInteger platformStatus = new MamaInteger();
			MamaString topic = new MamaString();
			MamaString statusText = new MamaString();
			
			// TODO BAD hardcoded, this is from the Solace bridge manual, but will be removed when OM publish cb is available
			msg.tryString(null, 12, topic);
			if (topic.getValue() == null) {
				return;
			}

			Set<MdsOmPublisher> pl = feedbackMap.get(topic.getValue());
			if (pl == null) {
				return;
			}

			msg.tryI32(null, 9, eventType);
			msg.tryString(null, 13, statusText);
			msg.tryI32(null, 14, mamaStatus);
			msg.tryI32(null, 15, platformStatus);

			Mama.log(MamaLogLevel.FINEST, "MdsOmPublisher::solacePublish: " + topic +
					" mamaStatus=" + mamaStatus + " platformStatus=" + platformStatus + " eventType=" + eventType);

			// Send feedback to each publisher of this symbol
			for (MdsOmPublisher pub : pl) {
				MdsOmPublisherCallback cb = pub.getCb();

	        	if (eventType.getValue() == 1) {
					if (cb != null) {
	           			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher::solacePublishFail: " + pub.getSubject() + " mamaStatus=" + mamaStatus + " platformStatus=" + platformStatus);
						cb.onPublishError(pub, (short) mamaStatus.getValue(), pub.getClosure());
					}
	        	}
			}
		} catch (Exception status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmPublisher::solacePublishFail: error " + status.toString());
		}
	}
}
