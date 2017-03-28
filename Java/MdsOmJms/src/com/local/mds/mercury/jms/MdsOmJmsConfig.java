package com.jpmorgan.mds.mercury.jms;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

public class MdsOmJmsConfig {
	private List<MdsOmJmsEnvConfig> configs = new ArrayList<MdsOmJmsEnvConfig>();
	
	private int statsInterval = 0;
	private boolean copyReceivedMessages = true;
	private boolean basicMode = true;
	private boolean print = false;
	private byte defaultPrecision = 6;
	
	// ---------------------------------------------------------------
	// For entitlements delegation
	private boolean entitlementsDelegation = false;
	private String entitlementsHost = "";
	
	private String user = "";
	private String pass = "";
	
	//Same defaults as Metafluent
	private SimpleDateFormat dateTimeFormat = new SimpleDateFormat("dd MMM yyyy HH:mm:ss");
	private SimpleDateFormat dateFormat = new SimpleDateFormat("dd MMM yyyy");
	private SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");

	
	public int getStatsInterval() {
		return statsInterval;
	}

	public void setStatsInterval(int statsInterval) {
		this.statsInterval = statsInterval;
	}

	public boolean isCopyReceivedMessages() {
		return copyReceivedMessages;
	}

	public void setCopyReceivedMessages(boolean copyReceivedMessages) {
		this.copyReceivedMessages = copyReceivedMessages;
	}

	public boolean isBasicMode() {
		return basicMode;
	}

	public void setBasicMode(boolean basicMode) {
		this.basicMode = basicMode;
	}

	public byte getDefaultPrecision() {
		return defaultPrecision;
	}

	public void setDefaultPrecision(byte defaultPrecision) {
		this.defaultPrecision = defaultPrecision;
	}

	public boolean isEntitlementsDelegation() {
		return entitlementsDelegation;
	}

	public void setEntitlementsDelegation(boolean entitlementsDelegation) {
		this.entitlementsDelegation = entitlementsDelegation;
	}

	public String getEntitlementsHost() {
		return entitlementsHost;
	}

	public void setEntitlementsHost(String entitlementsHost) {
		this.entitlementsHost = entitlementsHost;
	}

	public String getUser() {
		return user;
	}

	public void setUser(String user) {
		this.user = user;
	}

	public String getPass() {
		return pass;
	}

	public void setPass(String pass) {
		this.pass = pass;
	}

	public SimpleDateFormat getDateFormat() {
		return dateFormat;
	}

	public void setDateFormat(SimpleDateFormat dateFormat) {
		this.dateFormat = dateFormat;
	}

	public SimpleDateFormat getTimeFormat() {
		return timeFormat;
	}

	public void setTimeFormat(SimpleDateFormat timeFormat) {
		this.timeFormat = timeFormat;
	}

	public SimpleDateFormat getDateTimeFormat() {
		return dateTimeFormat;
	}

	public void setDateTimeFormat(SimpleDateFormat dateTimeFormat) {
		this.dateTimeFormat = dateTimeFormat;
	}

	public void setConfigs(List<MdsOmJmsEnvConfig> configs) {
		this.configs = configs;
	}

	public List<MdsOmJmsEnvConfig> getConfigs() {
		return configs;
	}
	
	public void addConfig(MdsOmJmsEnvConfig config) {
		configs.add(config);
	}

	public boolean isPrint() {
		return print;
	}

	public void setPrint(boolean print) {
		this.print = print;
	}
}
