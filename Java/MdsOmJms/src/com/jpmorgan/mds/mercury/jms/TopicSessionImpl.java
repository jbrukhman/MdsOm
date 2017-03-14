package com.jpmorgan.mds.mercury.jms;

import java.io.Serializable;

import javax.jms.BytesMessage;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageListener;
import javax.jms.MessageProducer;
import javax.jms.ObjectMessage;
import javax.jms.Queue;
import javax.jms.QueueBrowser;
import javax.jms.StreamMessage;
import javax.jms.TemporaryQueue;
import javax.jms.TemporaryTopic;
import javax.jms.TextMessage;
import javax.jms.Topic;
import javax.jms.TopicPublisher;
import javax.jms.TopicSession;
import javax.jms.TopicSubscriber;

import com.jpmorgan.mds.mercury.helper.MdsOm.MdsOmEnvType;
import com.jpmorgan.mds.mercury.helper.MdsOmEnv;
import com.jpmorgan.mds.mercury.helper.MdsOmStatus;
import com.wombat.mama.MamaMsg;

public class TopicSessionImpl implements TopicSession {
	private TopicConnectionImpl conn = null;

	protected TopicConnectionImpl getConn() {
		return conn;
	}

	protected void setConn(TopicConnectionImpl conn) {
		this.conn = conn;
	}

	@Override
	public void close() throws JMSException {
	}

	@Override
	public void commit() throws JMSException {
	}

	@Override
	public QueueBrowser createBrowser(Queue arg0) throws JMSException {
		return null;
	}

	@Override
	public QueueBrowser createBrowser(Queue arg0, String arg1)
			throws JMSException {
		return null;
	}

	@Override
	public BytesMessage createBytesMessage() throws JMSException {
		return null;
	}

	@Override
	public MessageConsumer createConsumer(Destination arg0) throws JMSException {
		return null;
	}

	@Override
	public MessageConsumer createConsumer(Destination arg0, String arg1)
			throws JMSException {
		return null;
	}

	@Override
	public MessageConsumer createConsumer(Destination arg0, String arg1,
			boolean arg2) throws JMSException {
		return null;
	}

	/**
	 * Used internally to get a msg with specific payload
	 * @param env
	 * @return
	 * @throws JMSException
	 */
	protected MapMessage createMapMessageFromPayload(MdsOmEnv env) throws JMSException {
		try {
			MapMessageImpl map = new MapMessageImpl();
			map.setEnv(env);
			map.setSession(this);
			map.setConfig(conn.getConfig());
			
			MamaMsg mamaMsg = env.getMamaMsg();
			map.setMamaMsg(mamaMsg);
			
			return map;
		} catch (MdsOmStatus e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}

	@Override
	public MapMessage createMapMessage() throws JMSException {
		try {
			// TODO need msg from correct env ???
			MapMessageImpl map = new MapMessageImpl();
			map.setConfig(conn.getConfig());
			map.setSession(this);

			MdsOmEnv env = conn.getOm().getEnv(MdsOmEnvType.SOLACE);		// TODO
			if (env == null) {
				env = conn.getOm().getEnv(MdsOmEnvType.TREP);
			}
			map.setEnv(env);
			
			// TODO optimize this in case they don't need a MamaMsg
			MamaMsg mamaMsg = env.getMamaMsg();
			map.setMamaMsg(mamaMsg);
			
			return map;
		} catch (MdsOmStatus e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}

	@Override
	public Message createMessage() throws JMSException {
		return createMapMessage();
	}

	@Override
	public ObjectMessage createObjectMessage() throws JMSException {
		return null;
	}

	@Override
	public ObjectMessage createObjectMessage(Serializable arg0)
			throws JMSException {
		return null;
	}

	@Override
	public MessageProducer createProducer(Destination arg0) throws JMSException {
		return null;
	}

	@Override
	public Queue createQueue(String arg0) throws JMSException {
		return null;
	}

	@Override
	public StreamMessage createStreamMessage() throws JMSException {

		return null;
	}

	@Override
	public TemporaryQueue createTemporaryQueue() throws JMSException {

		return null;
	}

	@Override
	public TextMessage createTextMessage() throws JMSException {

		return null;
	}

	@Override
	public TextMessage createTextMessage(String arg0) throws JMSException {

		return null;
	}

	@Override
	public int getAcknowledgeMode() throws JMSException {

		return 0;
	}

	@Override
	public MessageListener getMessageListener() throws JMSException {
		return null;
	}

	@Override
	public boolean getTransacted() throws JMSException {
		return false;
	}

	@Override
	public void recover() throws JMSException {
	}

	@Override
	public void rollback() throws JMSException {
	}

	@Override
	public void run() {
	}

	@Override
	public void setMessageListener(MessageListener arg0) throws JMSException {
	}

	@Override
	public TopicSubscriber createDurableSubscriber(Topic arg0, String arg1)
			throws JMSException {

		return null;
	}

	@Override
	public TopicSubscriber createDurableSubscriber(Topic arg0, String arg1,
			String arg2, boolean arg3) throws JMSException {
		return null;
	}

	@Override
	public TopicPublisher createPublisher(Topic topic) throws JMSException {
		TopicPublisherImpl pub = new TopicPublisherImpl(this, conn.getOm());
		pub.setTopic(topic);
		return pub;
	}

	@Override
	public TopicSubscriber createSubscriber(Topic topic) throws JMSException {
		TopicSubscriberImpl sub = new TopicSubscriberImpl(this, conn.getOm());
		sub.setTopic(topic);
		return sub;
	}

	@Override
	public TopicSubscriber createSubscriber(Topic topic, String arg1,
			boolean arg2) throws JMSException {
		TopicSubscriberImpl sub = new TopicSubscriberImpl(this, conn.getOm());
		sub.setTopic(topic);
		return sub;
	}

	@Override
	public TemporaryTopic createTemporaryTopic() throws JMSException {
		return null;
	}

	@Override
	public Topic createTopic(String name) throws JMSException {
		TopicImpl topic = new TopicImpl();
		topic.setTopicName(name);
		return topic;
	}

	@Override
	public void unsubscribe(String arg0) throws JMSException {
	}
}
