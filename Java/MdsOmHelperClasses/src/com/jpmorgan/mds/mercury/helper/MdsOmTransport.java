package com.jpmorgan.mds.mercury.helper;

import com.wombat.mama.MamaTransport;

public class MdsOmTransport {
	
	public static enum MdsOmTransportStatus {
		WAIT, SUCCESS, FAIL
	};
	
	private MdsOmTransportStatus connectionStatus = MdsOmTransportStatus.WAIT;
	private MamaTransport transport = null;
	
	public MdsOmTransport(MamaTransport t) {
		transport = t;
	}
	
	public MamaTransport getTransport() { return transport; }
	
	public String getName() { return transport.getName(); }
	
	public void setStatus(MdsOmTransportStatus status) { connectionStatus = status; } 
	
	public boolean isConnected() { return connectionStatus == MdsOmTransportStatus.SUCCESS; }
	
	public boolean isConnectWait() { return connectionStatus == MdsOmTransportStatus.WAIT; }

	public boolean isConnectFail() { return connectionStatus == MdsOmTransportStatus.FAIL; }
}
