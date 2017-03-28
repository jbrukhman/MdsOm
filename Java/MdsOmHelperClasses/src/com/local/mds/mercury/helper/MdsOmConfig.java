package com.jpmorgan.mds.mercury.helper;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.jpmorgan.mds.mercury.helper.MdsOm.MdsOmEnvType;

public class MdsOmConfig {

	public MdsOmEnvType type = MdsOmEnvType.UNKNOWN;
	public MdsOmTransportCallback transportCb = null;

	public String dictionaryFile = "";
	public String dictionarySource = "";
	
	public String username = "";
	
	// Either a single transport name or a list of transport names
	public String transportName = "";
	public List<String> transportNames = new ArrayList<String>();
	
	public String bridgeName = "";

	// Round robin transports
	public int roundRobinTransportCount = 0;

	// Map of transport to sources
	public Map<String, List<String>> sourceMap = new HashMap<String, List<String>>();
    
	int transportListIndex = 0;
	
	public MdsOmConfig() {
		clear();
	}
	
	public MdsOmConfig(MdsOmConfig config) {
		clear();
		type = config.type;
		transportName = config.transportName;
		dictionaryFile = config.dictionaryFile;
		dictionarySource = config.dictionarySource;
		roundRobinTransportCount = config.roundRobinTransportCount;
		bridgeName = config.bridgeName;
		transportCb = config.transportCb;
		username = config.username;
		sourceMap = new HashMap<String, List<String>>(config.sourceMap);
		transportNames = new ArrayList<String>(config.transportNames);
	}
	
	public void clear()
	{
		type = MdsOm.MdsOmEnvType.UNKNOWN;

		transportName = "";
		dictionaryFile = "";
		dictionarySource = "";
		bridgeName = "";
		username = "";
		
		transportNames.clear();
		sourceMap.clear();
		
		transportCb = null;
		// TODO transportTopicCb = null;

		roundRobinTransportCount = 0;
		transportListIndex = 0;
	}	

	public String toString()
	{
		return "transport=" + transportName + " dictionary=" + dictionaryFile + " dictionarySource=" + dictionarySource + " bridgeName=" + bridgeName + " username=" + username + " roundRobinTransportCount=" + roundRobinTransportCount;
	}
} 

