package com.jpmorgan.mds.mercury.jms;

import javax.jms.JMSException;
import javax.jms.JMSSecurityException;
import javax.jms.TopicConnection;

import com.jpmorgan.mds.mercury.helper.MdsOm;
import com.jpmorgan.mds.mercury.helper.MdsOmStatus;

public class MdsOmJmsPermissions {
	private MdsOm om = null;
	
	public MdsOmJmsPermissions(TopicConnection topicConnection) {
		MdsOm om = ((com.jpmorgan.mds.mercury.jms.TopicConnectionImpl) topicConnection).getOm();
		this.om = om;
	}
	
	// -------------------------------------------------------------
	// DACS Permissions - this is an extension to JMS
	public boolean checkSubscribePermissions(String user, String topic) throws JMSException
	{
		if (user == null || topic == null || user.length() == 0 || topic.length() == 0) {
			throw new JMSException("invalid user or topic");
		}
		
		try {
			return om.checkSubscribePermissions(user, topic);
		} catch (MdsOmStatus e) {
			throw new  JMSSecurityException(e.getMessage());
		}
	}
	
	public boolean checkPublishPermissions(String user, String topic) throws JMSException
	{
		if (user == null || topic == null || user.length() == 0 || topic.length() == 0) {
			throw new JMSException("invalid user or topic");
		}
		
		try {
			return om.checkPublishPermissions(user, topic);
		} catch (MdsOmStatus e) {
			throw new  JMSSecurityException(e.getMessage());
		}
	}
}
