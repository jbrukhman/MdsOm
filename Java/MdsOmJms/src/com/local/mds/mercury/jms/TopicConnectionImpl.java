package com.jpmorgan.mds.mercury.jms;

import javax.jms.ConnectionConsumer;
import javax.jms.ConnectionMetaData;
import javax.jms.Destination;
import javax.jms.ExceptionListener;
import javax.jms.JMSException;
import javax.jms.ServerSessionPool;
import javax.jms.Session;
import javax.jms.Topic;
import javax.jms.TopicConnection;
import javax.jms.TopicSession;

import com.jpmorgan.mds.mercury.helper.MdsOm;
import com.jpmorgan.mds.mercury.helper.MdsOmConfig;
import com.jpmorgan.mds.mercury.helper.MdsOmEnv;
import com.jpmorgan.mds.mercury.helper.MdsOmTransportCallback;
import com.wombat.mama.MamaTransport;

public class TopicConnectionImpl implements TopicConnection, MdsOmTransportCallback {
	private MdsOm om = null;
	private MdsOmJmsConfig config = null;
	private ExceptionListener listener = null;
	private String clientId = "";

	protected void setConfig(MdsOmJmsConfig config) {
		this.config = config;
	}
	
	protected MdsOmJmsConfig getConfig() {
		return config;
	}

	protected MdsOm getOm() {
		return om;
	}

	protected void setOm(MdsOm om) {
		this.om = om;
	}
	
	// -------------------------------------------------------------
	@Override
	public void close() throws JMSException {
		try {
			om.close();
		} catch (Exception e) {
			throw new JMSException(e.getMessage());		
		}
	}

	@Override
	public ConnectionConsumer createConnectionConsumer(Destination arg0,
			String arg1, ServerSessionPool arg2, int arg3) throws JMSException {
		return null;
	}

	@Override
	public Session createSession(boolean arg0, int arg1) throws JMSException {
		TopicSessionImpl session = new TopicSessionImpl();
		session.setConn(this);
		return session;
	}

	@Override
	public String getClientID() throws JMSException {
		return clientId;
	}

	@Override
	public ExceptionListener getExceptionListener() throws JMSException {
		return listener;
	}

	@Override
	public ConnectionMetaData getMetaData() throws JMSException {
		return null;
	}

	@Override
	public void setClientID(String clientId) throws JMSException {
		this.clientId = clientId;
	}

	@Override
	public void setExceptionListener(ExceptionListener listener)
			throws JMSException {
		this.listener = listener;
	}

	@Override
	public void start() throws JMSException {
		connectToOm();
	}

	@Override
	public void stop() throws JMSException {
		// TODO
	}

	@Override
	public ConnectionConsumer createConnectionConsumer(Topic arg0, String arg1,
			ServerSessionPool arg2, int arg3) throws JMSException {
		return null;
	}

	@Override
	public ConnectionConsumer createDurableConnectionConsumer(Topic arg0,
			String arg1, String arg2, ServerSessionPool arg3, int arg4)
			throws JMSException {
		return null;
	}

	@Override
	public TopicSession createTopicSession(boolean arg0, int arg1)
			throws JMSException {
		TopicSessionImpl session = new TopicSessionImpl();
		session.setConn(this);
		return session;
	}
	
	private void connectToOm() throws JMSException {
		try {
			if (config.getConfigs().size() == 0) {
				throw new JMSException("No envs to connect to");
			}
			
			// Check for entitlements delegation
			if (config.isEntitlementsDelegation()) {
				// Connect to entitlements system
			}
			
			// Startup OpenMAMA
			om = new MdsOm();
			om.setStatsInterval(config.getStatsInterval());
		
			// Add the app's configs
			for (MdsOmJmsEnvConfig cfg : config.getConfigs()) {
				MdsOmConfig c = new MdsOmConfig();
				c.transportCb = this;
				c.bridgeName = cfg.getBridgeName();
				c.transportName = cfg.getTransportName();
				if (cfg.getRoundRobinTransports() > 0) c.roundRobinTransportCount = cfg.getRoundRobinTransports();
				//This will apply to all configs
				c.username = config.getUser();
				om.addEnv(c);
			}
			
			// Open the envs
			om.open();
			
			// Start mama in the background
			om.start(true);
			
		} catch (Exception e) {
			throw new JMSException(e.getMessage());		
		}
	}

	// -----------------------------------------------------------------------------------
	@Override
	public void onDisconnect(MdsOmEnv env, MamaTransport transport,
			Object platformInfo) {
		if (listener != null && config.isBasicMode() == false) {
			listener.onException(new JMSException("Disconnected from env=" + env.getName() + " transport=" + transport.getName()));
		}
	}

	@Override
	public void onReconnect(MdsOmEnv env, MamaTransport transport,
			Object platformInfo) {
		if (listener != null && config.isBasicMode() == false) {
			listener.onException(new JMSException("Reconnected to env=" + env.getName() + " transport=" + transport.getName()));
		}
	}

	@Override
	public void onQuality(MdsOmEnv env, MamaTransport transport, short cause,
			Object platformInfo) {
		if (listener != null && config.isBasicMode() == false) {
			listener.onException(new JMSException("Quality event on env=" + env.getName() + " transport=" + transport.getName() + " cause=" + cause));
		}
	}

	@Override
	public void onConnect(MdsOmEnv env, MamaTransport transport,
			Object platformInfo) {
		if (listener != null && config.isBasicMode() == false) {
			listener.onException(new JMSException("Connected to env=" + env.getName() + " transport=" + transport.getName()));
		}
	}

	@Override
	public void onAccept(MdsOmEnv env, MamaTransport transport,
			Object platformInfo) {
	}

	@Override
	public void onAcceptReconnect(MdsOmEnv env, MamaTransport transport,
			Object platformInfo) {
	}

	@Override
	public void onPublisherDisconnect(MdsOmEnv env, MamaTransport transport,
			Object platformInfo) {
	}

	@Override
	public void onNamingServiceConnect(MdsOmEnv env, MamaTransport transport,
			Object platformInfo) {
	}

	@Override
	public void onNamingServiceDisconnect(MdsOmEnv env,
			MamaTransport transport, Object platformInfo) {
	}
}
