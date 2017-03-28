package com.jpmorgan.mds.mercury.jms;

import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MessageFormatException;
import javax.jms.Message;
import javax.jms.Topic;
import javax.jms.TopicPublisher;

import com.jpmorgan.mds.mercury.helper.MdsOm;
import com.jpmorgan.mds.mercury.helper.MdsOmEnv;
import com.jpmorgan.mds.mercury.helper.MdsOmPublisher;
import com.jpmorgan.mds.mercury.helper.MdsOmPublisherCallback;

public class TopicPublisherImpl implements TopicPublisher, MdsOmPublisherCallback {
	private MdsOmPublisher pub = null;
	private TopicSessionImpl session = null;
	private MdsOm om = null;
	private Topic topic = null;
	
	protected TopicSessionImpl getSession() {
		return session;
	}

	protected void setSession(TopicSessionImpl session) {
		this.session = session;
	}

	protected void setTopic(Topic topic) {
		this.topic = topic;
	}

	protected TopicPublisherImpl(TopicSessionImpl session, MdsOm om) {
		this.session = session;
		this.om = om;
	}

	@Override
	public void close() throws JMSException {
		pub.destroy();
		pub = null;
	}

	@Override
	public int getDeliveryMode() throws JMSException {
		return 0;
	}

	@Override
	public Destination getDestination() throws JMSException {
		return topic;
	}

	@Override
	public boolean getDisableMessageID() throws JMSException {
		return false;
	}

	@Override
	public boolean getDisableMessageTimestamp() throws JMSException {
		return false;
	}

	@Override
	public int getPriority() throws JMSException {
		return 0;
	}

	@Override
	public long getTimeToLive() throws JMSException {
		return 0;
	}

	@Override
	public void send(Message msg) throws JMSException {
	}

	@Override
	public void send(Destination arg0, Message arg1) throws JMSException {
	}

	@Override
	public void send(Message arg0, int arg1, int arg2, long arg3)
			throws JMSException {
	}

	@Override
	public void send(Destination arg0, Message arg1, int arg2, int arg3,
			long arg4) throws JMSException {
	}

	@Override
	public void setDeliveryMode(int arg0) throws JMSException {
	}

	@Override
	public void setDisableMessageID(boolean arg0) throws JMSException {
	}

	@Override
	public void setDisableMessageTimestamp(boolean arg0) throws JMSException {
	}

	@Override
	public void setPriority(int arg0) throws JMSException {
	}

	@Override
	public void setTimeToLive(long arg0) throws JMSException {
	}

	@Override
	public Topic getTopic() throws JMSException {
		return topic;
	}

	@Override
	public void publish(Message msg) throws JMSException {
		try {
			if (topic == null) {
				throw new JMSException("No topic found for published message");
			}
			
			if (pub == null) {
				pub = om.getPublisher(topic.getTopicName(),  this,  null);
			}
			
			MapMessageImpl m = (MapMessageImpl) msg;

			MdsOmEnv mapEnv = m.getEnv();
			MdsOmEnv pubEnv = pub.getEnv();
			if (mapEnv != pubEnv) {
				// TODO bad.
				// Pub is tick42, but msg is solace, need to swap.
				// This only occurs the first time a msg is published, after that it has the correct env.
				m.updateToNewEnv(pubEnv);
			}

			try {
				// Check for required field.
				// These will throw an exception if not found.
				m.getInt("MdMsgType");
			} catch (Exception e) {
				throw new MessageFormatException("Published message must have msgType: " + e.getMessage());
			}
			
			try {
				// Check for required field.
				// These will throw an exception if not found.
				m.getInt("MdMsgStatus");
			} catch (Exception e) {
				throw new MessageFormatException("Published message must have msgStatus: " + e.getMessage());
			}
			
			// Now publish the message
			pub.send(m.getMamaMsg());
		} catch (Exception e) {
			throw new JMSException(e.getMessage());
		}			
	}

	@Override
	public void publish(Topic topic, Message msg) throws JMSException {
		try {
			this.topic = topic;
			publish(msg);
		} catch (Exception e) {
			throw new JMSException(e.getMessage());
		}			
	}

	@Override
	public void publish(Message msg, int arg1, int arg2, long arg3)
			throws JMSException {
		publish(msg);
	}

	@Override
	public void publish(Topic topic, Message msg, int arg2, int arg3, long arg4)
			throws JMSException {
		publish(topic, msg);
	}

	// ---------------------------------------------------------------------------------------
	@Override
	public void onPublishCreate(MdsOmPublisher arg0, Object arg1) {
	}

	@Override
	public void onPublishDestroy(MdsOmPublisher arg0, Object arg1) {
	}

	@Override
	public void onPublishError(MdsOmPublisher arg0, short arg1, Object arg2) {
	}
}
