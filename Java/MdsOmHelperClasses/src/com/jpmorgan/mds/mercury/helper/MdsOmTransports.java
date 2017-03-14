package com.jpmorgan.mds.mercury.helper;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;

import com.jpmorgan.mds.mercury.helper.MdsOmTransport.MdsOmTransportStatus;
import com.wombat.mama.Mama;
import com.wombat.mama.MamaLogLevel;
import com.wombat.mama.MamaTransport;

public class MdsOmTransports {
	
	private MdsOmEnv env = null;
	private List<MdsOmTransport> transportList = new ArrayList<MdsOmTransport>();
	private Map<String, MdsOmTransport> transportMap = new Hashtable<String, MdsOmTransport>();
	int listIndex = 0;
	
	public MdsOmTransports() {
	}
	
	public void close() throws Exception {	
		try {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmTransports::close: " + env.getName() + " delete transports map count=" + transportMap.size());
			transportMap.clear();

			Mama.log(MamaLogLevel.NORMAL, "MdsOmTransports::close: " + env.getName() + " delete transports list count=" + transportList.size());
			transportList.clear();

			Mama.log(MamaLogLevel.NORMAL, "MdsOmTransports::close: " + env.getName());
		} catch (Exception status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmTransports::close: error " + env.getName() + " " + status.toString());
			// Send original exception reference back to caller
			throw status;
		}
	}
	
	public void setEnv(MdsOmEnv env) { this.env = env; }

	public boolean isMap() { return transportMap.size() > 0; }

	public int mapSize() { return transportMap.size(); }

	public int listSize() { return transportList.size(); }

	public void addTransport(MamaTransport transport) {
		// Add to list
		transportList.add(new MdsOmTransport(transport));
	}

	public void addTransport(MamaTransport transport, String source) {
		// Add to map
		transportMap.put(source, new MdsOmTransport(transport));
	}

	public void setStatus(MamaTransport transport, MdsOmTransportStatus status)
	{
		if (isMap()) {
			for (MdsOmTransport t : transportMap.values()) {
				if (t.getTransport() == transport) {
					t.setStatus(status);
					return;
				}
			}
		} else {
			for (MdsOmTransport t : transportList) {
				if (t.getTransport() == transport) {
					t.setStatus(status);
					return;
				}
			}
		}
		Mama.log(MamaLogLevel.ERROR,"MdsOmTransports::setStatus: did not find transport " + transport.getName());
		printTransports();
	}

	public void printTransports() {
		Mama.log(MamaLogLevel.NORMAL, "MdsOmTransports::printTransports: " + env.getName() + " map=" + transportMap.size() + " list=" + transportList.size());
			
		if (isMap()) {
			for (MdsOmTransport t : transportMap.values()) {
				Mama.log(MamaLogLevel.NORMAL, "MdsOmTransports::printTransports: " + t.getTransport().getName());
			}
		} else {
			for (MdsOmTransport t : transportList) {
				Mama.log(MamaLogLevel.NORMAL, "MdsOmTransports::printTransports: " + t.getTransport().getName());
			}
		}
	}

	public MamaTransport getTransport(String source) throws MdsOmStatus {
		if (transportMap.size() == 0 && transportList.size() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmTransports::getTransport(source): error " + env.getName() + " both collections are empty");
			throw new MdsOmStatus(MdsOmStatus.NO_TRANSPORTS_AVAILABLE);
		}

		if (transportList.size() > 0) {
			return getTransport();
		} else {
			MdsOmTransport t = transportMap.get(source);
			if (t != null) return t.getTransport();
			for (MdsOmTransport t1 : transportMap.values()) {
				return t1.getTransport();
			}
		}
		return null;
	}

	public MamaTransport getTransport() throws MdsOmStatus {
		if (transportList.size() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmTransports::getTransport(): error " + env.getName() + " list is empty");
			throw new MdsOmStatus(MdsOmStatus.NO_TRANSPORTS_AVAILABLE);
		}

		MdsOmTransport t = transportList.get(listIndex);
		if (++listIndex >= transportList.size()) {
			listIndex = 0;
		}
		if (t == null) {
			return transportList.get(0).getTransport();
		} else {
			return t.getTransport();
		}
	}

	public void getTransports(List<MamaTransport> tlist) {
		if (transportList.size() == 0 && transportMap.size() == 0) {
			return;
		}

		if (!isMap()) {
			for (MdsOmTransport t : transportList) {
				tlist.add(t.getTransport());
			}
		} else {
			for (MdsOmTransport t : transportMap.values()) {
				tlist.add(t.getTransport());
			}
		}
	}


	public boolean isConnected() {
		boolean ok = true;
		if (!isMap()) {
			for (MdsOmTransport t : transportList) {
				if (t.isConnected() == false) ok = false;
			}
		} else {
			for (MdsOmTransport t : transportMap.values()) {
				if (t.isConnected() == false) ok = false;
			}
		}
		return ok;
	}


	public boolean isConnectWait() {
		boolean waiting = false;
		if (!isMap()) {
			for (MdsOmTransport t : transportList) {
				if (t.isConnectWait() == true) waiting = true;
			}
		} else {
			for (MdsOmTransport t : transportMap.values()) {
				if (t.isConnectWait() == true) waiting = true;
			}
		}
		return waiting;
	}


	public boolean isConnectFail() {
		boolean fail = false;
		if (!isMap()) {
			for (MdsOmTransport t : transportList) {
				if (t.isConnectFail() == true) fail = true;
			}
		} else {
			for (MdsOmTransport t : transportMap.values()) {
				if (t.isConnectFail() == true) fail = true;
			}
		}
		return fail;
	}
}
