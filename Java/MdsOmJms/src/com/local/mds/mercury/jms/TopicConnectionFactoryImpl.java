package com.jpmorgan.mds.mercury.jms;

import java.util.ArrayList;
import java.util.List;

import javax.jms.Connection;
import javax.jms.JMSException;
import javax.jms.TopicConnection;
import javax.jms.TopicConnectionFactory;

import com.jpmorgan.mds.mercury.helper.MdsOm;
import com.wombat.mama.Mama;

public class TopicConnectionFactoryImpl implements TopicConnectionFactory {
	private MdsOmJmsConfig config = new MdsOmJmsConfig();
	private String bridgeName = "";
	public TopicConnectionFactoryImpl(MdsOmJmsConfig config) {
		//Env check is handled in TopicConnectionImpl.connectToOm()		
		//no need to throw exception
		this.config = config;
	}
	public TopicConnectionFactoryImpl() throws JMSException {
		throw new JMSException("Transport or MdsOmConfig needs to be specified.");
	}
	
	public TopicConnectionFactoryImpl(String transport) throws JMSException {
		config = new MdsOmJmsConfig();
		MdsOmJmsEnvConfig cfg;
		List<String> transports = new ArrayList<String>();
		//Generate list of transports in the event more than one is provided
		transports = getTransports(transport);
		
		config.setStatsInterval(0);
		config.setBasicMode(true); //if set to false it will break
		config.setCopyReceivedMessages(false);
	
		for(String trans : transports){
			if(!getProperty(trans))
				throw new JMSException("Transport "+transport+" does not exist in mama.properties");

			cfg = new MdsOmJmsEnvConfig();
			cfg.setBridgeName(bridgeName); //check for null
			cfg.setTransportName(trans);
			config.addConfig(cfg);
		}
	}
	
	@Override
	public Connection createConnection() throws JMSException {
		TopicConnectionImpl c = new TopicConnectionImpl();
		c.setConfig(config);
		return c;
	}

	@Override
	public Connection createConnection(String username, String password)
			throws JMSException {
		// TODO put username into mama properties
		TopicConnectionImpl c = new TopicConnectionImpl();
		config.setUser(username);
		config.setPass(password);
		c.setConfig(config);
		return c;
	}

	@Override
	public TopicConnection createTopicConnection() throws JMSException {
		TopicConnectionImpl c = new TopicConnectionImpl();
		c.setConfig(config);
		return c;
	}

	@Override
	public TopicConnection createTopicConnection(String username, String password)
			throws JMSException {
		// TODO put username into mama properties
		TopicConnectionImpl c = new TopicConnectionImpl();
		config.setUser(username);
		config.setPass(password);
		c.setConfig(config);
		return c;
	}
	
	//Return list of parsed transports in list of strings
	private List<String> getTransports(String transports){
		List<String> ret = new ArrayList<String>();
		String[] trans = transports.split(",");

		for(String token : trans)
			ret.add((String) token);

		return ret;
	}
	
	private boolean getProperty(String prop){
		if(Mama.getProperty("mama."+MdsOm.solaceBridgeName+".transport."+prop+".session_host") != null){
			bridgeName = MdsOm.solaceBridgeName;
			return true;
		}else if(Mama.getProperty("mama."+MdsOm.tick42BridgeName+".transport."+prop+".hosts") != null){
			bridgeName = MdsOm.tick42BridgeName;
			return true;
		}else if(Mama.getProperty("mama."+MdsOm.mamacacheBridgeName+".transport."+prop+".hosts") != null){
			bridgeName = MdsOm.mamacacheBridgeName;
			return true;
		}
		
		return false;
	}
}
