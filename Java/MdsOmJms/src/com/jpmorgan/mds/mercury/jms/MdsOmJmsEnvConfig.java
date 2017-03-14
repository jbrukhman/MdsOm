package com.jpmorgan.mds.mercury.jms;

public class MdsOmJmsEnvConfig {
	private String bridgeName = "";
	private String transportName = "";
	private int roundRobinTransports;
	
	public String getBridgeName() {
		return bridgeName;
	}
	public void setBridgeName(String bridgeName) {
		this.bridgeName = bridgeName;
	}
	public String getTransportName() {
		return transportName;
	}
	public void setTransportName(String transportName) {
		this.transportName = transportName;
	}

	public int getRoundRobinTransports() {
		return roundRobinTransports;
	}

	public void setRoundRobinTransports(int roundRobinTransports) {
		this.roundRobinTransports = roundRobinTransports;
	}
}
