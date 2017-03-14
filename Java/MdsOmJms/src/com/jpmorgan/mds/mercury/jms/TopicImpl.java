package com.jpmorgan.mds.mercury.jms;

import javax.jms.JMSException;
import javax.jms.Topic;

public class TopicImpl implements Topic {
	private String topicName = null;

	protected void setTopicName(String topicName) {
		this.topicName = topicName;
	}

	@Override
	public String getTopicName() throws JMSException {
		return topicName;
	}
}
