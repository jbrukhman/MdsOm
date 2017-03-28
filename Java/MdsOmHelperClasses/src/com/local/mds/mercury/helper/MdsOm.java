package com.jpmorgan.mds.mercury.helper;

import java.io.FileNotFoundException;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.Semaphore;

import com.wombat.mama.Mama;
import com.wombat.mama.MamaLogLevel;
import com.wombat.mama.MamaQueue;
import com.wombat.mama.MamaStartBackgroundCallback;
import com.wombat.mama.MamaSubscriptionCallback;
import com.wombat.mama.MamaTimer;
import com.wombat.mama.MamaTimerCallback;

public class MdsOm implements MamaTimerCallback, MamaStartBackgroundCallback {
	// ENVIRONMENTS
	private MdsOmEnv idp = null;
	private MdsOmEnv rmds = null;
	private MdsOmEnv firefly = null;
	private List<MdsOmEnv> envs = new ArrayList<MdsOmEnv>();
	
	private MdsOmDacs dacs = null;
	   
	// For logging
	String logFile = null;										// log file name, default is NULL, which is stderr
	
	private int statsInterval = 0;								// statistics interval, if 0 then disable stats
	private final Semaphore doneSem = new Semaphore(0, true);	// used to synchronize the shutdown
	private MamaTimer statsTimer;

	private boolean allBg = false;								// true = all envs are started in background
	private boolean didStart = false;							// true = called start, so we can call stop
	private int waits = 30;										// used to wait for transport/dictionaries/etc
	private boolean fullTimeout = false;						// wait for full wait timeout to return error to app
	
	public final static String solaceBridgeName = "solace";
	public final static String tick42BridgeName = "tick42rmds";
	public final static String mamacacheBridgeName = "wmw";
	
	// Enum for env types
	public static enum MdsOmEnvType {
		MERCURY,						// Mercury/IDP/Solace
		SOLACE,							// Solace
		TREP,							// TREP/Tick42
	   	MAMACACHE,						// SRLabs Firefly/MamaCache/Wombat
	   	WMW,							// SRLabs
	   	UNKNOWN
	}

	public MdsOm() {
		init(null, MamaLogLevel.NORMAL);
	}
	
	public MdsOm(String logFile, MamaLogLevel level) {
		init(logFile, level);
	}
	
	public boolean isFullTimeout() {
		return fullTimeout;
	}

	public void setFullTimeout(boolean fullTimeout) {
		this.fullTimeout = fullTimeout;
	}

	public void setWaits(int waits) {
		Mama.log(MamaLogLevel.NORMAL, "MdsOm::setWaits: waits=" + waits);
		this.waits = waits;
	}
	
	public int getWaits() {
		return waits;
	}
	
	// --- PERMISSIONS ---------------------------------------------------------------
	public boolean checkSubscribePermissions(String user, String topic) throws MdsOmStatus
	{
		if (user == null || topic == null || user.length() == 0 || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.checkSubscribePermissions: invalid user or topic");
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
		
		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return checkSubscribePermissions(user, s.x, s.y);
	}
	
	public boolean checkSubscribePermissions(String user, String source, String symbol) throws MdsOmStatus
	{
		if (user == null || source == null || symbol == null || user.length() == 0 || source.length() == 0 || symbol.length() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.checkSubscribePermissions: invalid user or source/symbol");
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
		
		if (dacs == null) {
			initDacs();
		}
		
		return dacs.checkSubscribePermission(user,  source, symbol);
	}
	
	public boolean checkPublishPermissions(String user, String topic) throws MdsOmStatus
	{
		if (user == null || topic == null || user.length() == 0 || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.checkPublishPermissions: invalid user or topic");
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
		
		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return checkPublishPermissions(user, s.x, s.y);
	}
	
	public boolean checkPublishPermissions(String user, String source, String symbol) throws MdsOmStatus
	{
		if (user == null || source == null || symbol == null || user.length() == 0 || source.length() == 0 || symbol.length() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.checkPublishPermissions: invalid user or source/symbol");
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
		
		if (dacs == null) {
			initDacs();
		}
		
		return dacs.checkPublishPermission(user,  source, symbol);
	}
	
	private void initDacs() throws MdsOmStatus
	{
		dacs = new MdsOmDacs();
		
		String dacsHostsProperty = "mama.entitlement.dacs.hosts";
		String dacsHosts = Mama.getProperty(dacsHostsProperty);
		if (dacsHosts == null) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.initDacs: did not find dacs host property: " + dacsHostsProperty);
			throw new MdsOmStatus(MdsOmStatus.DACS_NO_HOSTS);
		}
		
		Mama.log(MamaLogLevel.NORMAL, "MdsOM.dacsInit: hosts=" + dacsHosts);
		String hosts[] = dacsHosts.split(",");
		int count = 0;
		for (String host : hosts) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOM.dacsInit: add host=" + host);
			dacs.addHost(host.trim());
			count++;
		}
		if (count == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.initDacs: did not find dacs hosts in property=" + dacsHostsProperty + " hosts='" + dacsHosts + "'");
			throw new MdsOmStatus(MdsOmStatus.DACS_NO_HOSTS);
		}
		
		boolean ret = dacs.open();
		if (!ret) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.initDacs: dacs open and host connections failed");
			throw new MdsOmStatus(MdsOmStatus.DACS_CONNECT_FAILED);			
		}
	}
	// --- PERMISSIONS ---------------------------------------------------------------
	
	public String getBridgeName(MdsOmEnvType type){
		switch(type){
		case MAMACACHE:
		case WMW:
			return mamacacheBridgeName;
		case MERCURY:
		case SOLACE:
			return solaceBridgeName;
		case TREP:
			return tick42BridgeName;
		case UNKNOWN:
		default:
			return "";
		}
	}
	
	public void init(String logFile, MamaLogLevel level) {
		if (logFile != null) {
			this.logFile = logFile;
		}
		
		// This turns on logging, and will be redone w/ mama.properties when open is called
		if (logFile != null) {
			try {
				Mama.enableLogging(MamaLogLevel.getLevel(level), logFile);
			} catch (FileNotFoundException e) {
				System.err.printf("MdsOm: cannot open log file %s\n",logFile);
			}
		} else {
			Mama.enableLogging(MamaLogLevel.getLevel(level));
		}
		
		String envVars[] = {
			"WOMBAT_PATH",
			"OPENMAMA_HOME",
			"LD_LIBRARY_PATH",
			"PATH",
			"CLASSPATH",
			"USER",
			"USERNAME",
			"LOGNAME",
			"HOSTNAME",
			"COMPUTERNAME",
			"JAVA_HOME",
		};

		for (String envVar : envVars) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOm: " + envVar + "=" + System.getenv(envVar));			
		}
		
		String username = Mama.getUserName();
		Mama.log(MamaLogLevel.NORMAL, "MdsOm: mama_getUserName=" + username);
		Mama.log(MamaLogLevel.NORMAL, "MdsOm: waits=" + waits);
	}
	
	public MdsOmEnv addEnv(MdsOmConfig config) throws MdsOmStatus {
		if (config.roundRobinTransportCount > 0 && config.sourceMap.size() > 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm.addEnv: " + config.type + " invalid balance roundRobin=" + config.roundRobinTransportCount + " and list=" + config.sourceMap.size());
			throw new MdsOmStatus(MdsOmStatus.INVALID_CONFIG);
		}
		   
		if (config.bridgeName != null) {
			if (config.bridgeName.equalsIgnoreCase(solaceBridgeName)) {
				config.type = MdsOmEnvType.MERCURY;
			} else if (config.bridgeName.equalsIgnoreCase(tick42BridgeName)) {
				config.type = MdsOmEnvType.TREP;			
			}
		}
		
		MdsOmEnv env = null;
		switch (config.type) {
		case MERCURY:
		case SOLACE:
			if (idp != null) {
				Mama.log(MamaLogLevel.ERROR, "MdsOm.addEnv: already have Mercury env added");
				throw new MdsOmStatus(MdsOmStatus.INVALID_CONFIG);
			}

			if (config.roundRobinTransportCount > 1 || config.sourceMap.size() > 0) {
				Mama.log(MamaLogLevel.ERROR, "MdsOm.addEnv: " + config.type + " Mercury/Solace supports one transport per name -  roundRobin=" + config.roundRobinTransportCount + " and list=" + config.sourceMap.size());
				throw new MdsOmStatus(MdsOmStatus.INVALID_CONFIG);
			}
			env = idp = new MdsOmEnv();
			break;
		case TREP:
			if (rmds != null) {
				Mama.log(MamaLogLevel.ERROR, "MdsOm.addEnv: already have RMDS/TREP env added");
				throw new MdsOmStatus(MdsOmStatus.INVALID_CONFIG);
			}
			env = rmds = new MdsOmEnv();
			break;
		case MAMACACHE:
		case WMW:
			if (firefly != null) {
				Mama.log(MamaLogLevel.ERROR, "MdsOm.addEnv: already have FIREFLY env added");
				throw new MdsOmStatus(MdsOmStatus.INVALID_CONFIG);
			}
			env = firefly = new MdsOmEnv();
			break;
		default:
			Mama.log(MamaLogLevel.ERROR,"MdsOm.addEnv: invalid env requested type=" + config.type);
			throw new MdsOmStatus(MdsOmStatus.INVALID_ENV);
		}
		
		boolean enableStats = statsInterval > 0 ? true : false;
		
		try {
			env.init(config, enableStats);
		} catch (MdsOmStatus e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.addEnv: error " + env.getName() + " " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
		envs.add(env);
		
		return env;
	}
	
	public MdsOmEnv getEnv(MdsOmEnvType type) throws MdsOmStatus {
	   switch(type) {
	   case MERCURY:
	   case SOLACE:
		   return idp;
	   case TREP:
		   return rmds;
	   case MAMACACHE:
	   case WMW:
		   return firefly;
	   default:
		   Mama.log(MamaLogLevel.ERROR,"MdsOm.getEnv: invalid env requested type=" + type);
		   throw new MdsOmStatus(MdsOmStatus.INVALID_ENV);
	   }
	} 
	
	public MdsOmEnv getEnvFromSymbol(String symbol) throws MdsOmStatus {
		if (symbol == null) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm::getEnvFromSymbol: null arg passed topic" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		// Figure out which env handles this source
		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(symbol);
		String source = s.getX();
		MdsOmEnv env = findEnv(source);
		return env;
	}

	// Get a queue to use for creating timers
	public MamaQueue getTimerQueue() throws MdsOmStatus {
		for (MdsOmEnv e : envs) {
			return e.getQueue();
		}
		Mama.log(MamaLogLevel.ERROR,"MdsOm.getTimerQueue: no queues available envs=" + envs.size());
		throw new MdsOmStatus(MdsOmStatus.NO_QUEUES_AVAILABLE);
	}
	
	// SNAPSHOTS ---------------------------------------------------
	public MdsOmSubscription snap(String topic, MamaSubscriptionCallback cb, Object closure) throws MdsOmStatus {
		if (topic == null || cb == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.snap: null arg passed in cb=" + cb + " topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
	   
		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return snap(s.getX(), s.getY(), cb, closure, false);
	}
	
	public MdsOmSubscription snap(String topic, MamaSubscriptionCallback cb, Object closure, boolean setup) throws MdsOmStatus {
		if (topic == null || cb == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.snap: null arg passed in cb=" + cb + " topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
	   
		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return snap(s.getX(), s.getY(), cb, closure, setup);
	}
   
	public MdsOmSubscription snap(String source, String symbol, MamaSubscriptionCallback cb, Object closure, boolean setup) throws MdsOmStatus {
		if (source == null || symbol == null || source.length() == 0 || symbol.length() == 0 || cb == null) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.snap: null arg passed in cb=" + cb + " source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
		
		try {
			// Figure out which env handles this source
			MdsOmEnv env = findEnv(source);	
			if (env == null) {
				Mama.log(MamaLogLevel.ERROR,"MdsOm.snap: cannot find source=" + source + " symbol=" + symbol);
				throw new MdsOmStatus(MdsOmStatus.CANNOT_FIND_SOURCE_IN_CONFIG);
			}

			return env.snap(source, symbol, cb, closure, setup);
			
		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.snap: error " + source + "." + symbol + " " + MdsOmUtil.getStackTrace(status));
			throw status;
		}
	}
	
	// SUBSCRIPTION ------------------------------------------------
	public MdsOmSubscription subscribe(String topic, MamaSubscriptionCallback cb, Object closure) throws MdsOmStatus {
		if (topic == null || cb == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.subscribe: null arg passed in cb=" + cb + " topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
	   
		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return subscribe(s.getX(), s.getY(), cb, closure, false);
	}
	
	public MdsOmSubscription subscribe(String topic, MamaSubscriptionCallback cb, Object closure, boolean setup) throws MdsOmStatus {
		if (topic == null || cb == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.subscribe: null arg passed in cb=" + cb + " topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
	   
		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return subscribe(s.getX(), s.getY(), cb, closure, setup);
	}
   
	public MdsOmSubscription subscribe(String source, String symbol, MamaSubscriptionCallback cb, Object closure, boolean setup) throws MdsOmStatus {
		if (source == null || symbol == null || source.length() == 0 || symbol.length() == 0 || cb == null) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.subscribe: null arg passed in cb=" + cb + " source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
		
		try {
			// Figure out which env handles this source
			MdsOmEnv env = findEnv(source);	
			if (env == null) {
				Mama.log(MamaLogLevel.ERROR,"MdsOm.subscribe: cannot find source=" + source + " symbol=" + symbol);
				throw new MdsOmStatus(MdsOmStatus.CANNOT_FIND_SOURCE_IN_CONFIG);
			}

			return env.subscribe(source, symbol, cb, closure, setup);
			
		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.subscribe: error " + source + "." + symbol + " " + MdsOmUtil.getStackTrace(status));
			throw status;
		}
	}

   
	public MdsOmEnv findEnv(String source) {
		if (envs.size() == 1) {
			// Only 1 env, just default to that one
			return envs.get(0);
		} else {
			// Find which env handles this
			for (MdsOmEnv e : envs) {
				boolean b = e.findSourceList(source);
				if (b) return e;
			}
		}
		return null;
	}
	
	// PUBLISHERS -------------------------------------------------------------------------------------------------------
	public MdsOmPublisher getPublisher(String topic, MdsOmPublisherCallback cb, Object closure) throws MdsOmStatus {
		if (topic == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.getPublisher: null arg passed in topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return getPublisher(s.getX(), s.getY(), cb, closure);
	}

	public MdsOmPublisher getPublisher(String source, String symbol, MdsOmPublisherCallback cb, Object closure) throws MdsOmStatus {
		if (source == null || symbol == null || source.length() == 0 || symbol.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOm.getPublisher: null arg passed in source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		try {
			MdsOmEnv env = findEnv(source);
			if (env == null) {
				Mama.log(MamaLogLevel.ERROR,"MdsOm.getPublisher: cannot find source=" + source + " symbol=" + symbol);
				throw new MdsOmStatus(MdsOmStatus.CANNOT_FIND_SOURCE_IN_CONFIG);
			}

			return env.getPublisher(source, symbol, cb, closure);

		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.getPublisher: error " + source + "." + symbol + " " + MdsOmUtil.getStackTrace(status));
			throw status;
		}
	}
	
	public void open() throws MdsOmStatus, Exception {
		open(null, null);
	}
	
	// Open the envs
	public void open(String path, String filename) throws MdsOmStatus, Exception {
		try {
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: envs=" + envs.size() + " path=" + path + " filename=" + filename + " waits=" + waits + " fullTo=" + fullTimeout);
			
			if (path != null || filename != null) {
				Mama.setProperty(path, filename);
			}
			
			if (envs.size() == 0) {
				Mama.log(MamaLogLevel.ERROR,"MdsOm.open: no environments configured");
				throw new MdsOmStatus(MdsOmStatus.NO_ENVS);
			}
			
			for (MdsOmEnv e : envs) {
				Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: env " + e.getName());
			}

			// -----------------------------------------------------------
			// Load bridges
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: load envs");
			for (MdsOmEnv e : envs) {
				Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: load env " + e.getName());
				e.load();
			}

			// -----------------------------------------------------------
			// Open MAMA
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: open mama");
			Mama.open();

			// -----------------------------------------------------------
			// Open bridges
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: open envs");
			for (MdsOmEnv e : envs) {
				Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: open env " + e.getName());
				e.open();
			}

			int localWaits = 0;
			while (true) {
				int waiting = 0;
				int countConnected = 0;
				for (MdsOmEnv e : envs) {
					if (e.isConnectWait()) waiting++;
					if (e.isConnected()) countConnected++;
				}
				
				if (countConnected == envs.size()) {
					// All OK
					break;
				}
				
				if (!fullTimeout && waiting == 0) {
					// All of the connections failed and not waiting for full timeout
					break;
				}
				
				localWaits++;
				Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: waiting for connection count=" + localWaits);
				if (waits > 0 && localWaits > waits) {
					for (MdsOmEnv e : envs) {
						if (e.isConnectWait()) {
							Mama.log(MamaLogLevel.ERROR,"MdsOm.open: env did not connect " + e.getName() + " lw=" + localWaits +" w=" + waits);
						}
					}
					throw new MdsOmStatus(MdsOmStatus.BRIDGE_TIMEOUT);
				}
				
				Thread.sleep(2000);
			}

			boolean isConnectError = false;
			for (MdsOmEnv e : envs) {
				if (e.isConnectFail()) {
					isConnectError = true;
					Mama.log(MamaLogLevel.ERROR, "MdsOm.open: env failed to open " + e.getName());
				}
			}
			if (isConnectError) {
				throw new MdsOmStatus(MdsOmStatus.BRIDGE_DID_NOT_CONNECT);
			}
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: all envs connected");

			// -----------------------------------------------------------
			// Load dictionaries
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: load dictionaries");
			for (MdsOmEnv e : envs) {
				Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: load dictionary " + e.getName());
				e.loadDictionary();
			}

			localWaits = 0;
			while (true) {
				int waiting = 0;
				for (MdsOmEnv e : envs) {
					if (e.isDictionaryWait()) waiting++;
				}
				if (waiting == 0) {
					break;
				}
				localWaits++;
				Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: waiting for dictionary count=" + localWaits);

				if (localWaits > waits) {
					for (MdsOmEnv e : envs) {
						if (e.isConnectWait()) {
							Mama.log(MamaLogLevel.ERROR,"MdsOm.open: dictionary did not load " + e.getName());
						}
					}
					throw new MdsOmStatus(MdsOmStatus.DICTIONARY_TIMEOUT);
				}
				Thread.sleep(2000);
			}

			boolean isDictionaryConnectError = false;
			for (MdsOmEnv e : envs) {
				if (e.isDictionaryFail()) {
					isDictionaryConnectError = true;
					Mama.log(MamaLogLevel.ERROR,"MdsOm.open: env failed to load dictionary " + e.getName());
				}
			}
			if (isDictionaryConnectError) {
				throw new MdsOmStatus(MdsOmStatus.DICTIONARY_DID_NOT_CONNECT);
			}
			
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.open: all envs OK");

		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.open: error " + MdsOmUtil.getStackTrace(status));
			throw status;
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.open: error " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}

	// Start OpenMama, this blocks
	public void start() throws MdsOmStatus, Exception {
		start(false);
	}
	
	public void start(boolean allBg) throws MdsOmStatus, Exception {
		try {
			this.allBg = allBg;
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.start: allBg=" + allBg + " size=" + envs.size());
			
			if (envs.size() == 0) {
				Mama.log(MamaLogLevel.ERROR,"MdsOm.start: no environments configured");
				throw new MdsOmStatus(MdsOmStatus.NO_ENVS);
			}

			// -----------------------------------------------------------
			// Start background bridges
			didStart = true; 		//note that we called start, so we can call stop later
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.start: start bg envs count=" + envs.size());
			int count = 0;
			MdsOmEnv fgBridge = null;
			for (MdsOmEnv e : envs) {
				if (allBg == false && ++count == envs.size()) {
					// Keep last bridge in list for foreground
					fgBridge = e;
				} else {
					Mama.log(MamaLogLevel.NORMAL,"MdsOm.start: start bg env " + e.getBridgeName());
					Mama.startBackground(e.getBridge(), this);
				}
			}

			if (statsInterval > 0) {
				// TODO get a queue for just the timer so the timer events are not stuck behind data
				MamaQueue timerQueue = getTimerQueue();
				statsTimer = new MamaTimer();
				statsTimer.create(timerQueue, this, statsInterval, null);
			}

			// -----------------------------------------------------------
			// Check fg/bg mode
			if (allBg) {
				Mama.log(MamaLogLevel.NORMAL,"MdsOm.start: all background bridges started, returning");
				return;
			}
			
			// -----------------------------------------------------------
			// This blocks
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.start: start fg env " + fgBridge.getBridgeName());
			Mama.start(fgBridge.getBridge());

			// -----------------------------------------------------------
			// Stopped, clean up
			Mama.log(MamaLogLevel.NORMAL,"MdsOm.start: start fg env " + fgBridge.getBridgeName() + " exited");

			cleanup();

			doneSem.release(1);			// TODO check this 

		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.start: error " + MdsOmUtil.getStackTrace(status));
			throw status;
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.start: error " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}

	void cleanup() throws Exception
	{
		try {
			// -----------------------------------------------------------
			// Stopped, clean up
			if (statsInterval > 0) {
				statsTimer.destroy();
			}

			for (MdsOmEnv e : envs) {
				e.close();
			}

			Thread.sleep(1000);
			
			// TODO make sure this is set correctly
			if (didStart) {
				Mama.log(MamaLogLevel.NORMAL, "MdsOm::cleanup: close envs count=" + envs.size());
				for (MdsOmEnv env : envs) {
					Mama.log(MamaLogLevel.NORMAL, "MdsOm::cleanup: close " + env.getName());
					env.close();
				}
			}

			Mama.log(MamaLogLevel.NORMAL, "MdsOm::cleanup: Mama::close");
			Mama.close();

			Mama.log(MamaLogLevel.NORMAL, "MdsOm::cleanup: all done");

		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.cleanup: error " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}
	
	public void stopQueues() throws Exception {
		try {
			Mama.log(MamaLogLevel.NORMAL, "MdsOm.stopQueues: close envs count=" + envs.size());
			for (MdsOmEnv e : envs) {
				e.stopQueues();
			}
		} catch(Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.stopQueues: error " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}
	
	// Close OpenMama
	public void close() throws MdsOmStatus, Exception {
		try {
			Mama.log(MamaLogLevel.NORMAL, "MdsOm.close:");
			
			stopQueues();
			
			if (didStart) {
				Mama.log(MamaLogLevel.NORMAL, "MdsOm.close: stopAll");
				for (MdsOmEnv e : envs) {
					Mama.stop(e.getBridge());
				}
			} else {
				Mama.log(MamaLogLevel.NORMAL, "MdsOm.close: did not call start, not calling stopAll");
			}
			
			if (!didStart || allBg) {
				cleanup();
			} else {
				// Ran with 1 brdige in fg, wait for it to exit
				Mama.log(MamaLogLevel.NORMAL, "MdsOm.close: wait for sem");
				doneSem.acquire(1);			// TODO wait for cleanup to finish on start thread
			}

			Mama.log(MamaLogLevel.NORMAL,"MdsOm.close: exit");
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.close: error " + MdsOmUtil.getStackTrace(e));
			throw e;
		}
	}

	// Set the statistics interval, default is 30 secs
	// Set to 0 to turn off stats
	public void setStatsInterval(int statsInterval) {
		this.statsInterval = statsInterval;
	}

	// ----------------------------------------------------------
	// MamaStartCallback
	public void onStartComplete(int status) {
		// Mama.log(MamaLogLevel.NORMAL,"MdsOm.onStartComplete: status " + status);
	}
   
	// ----------------------------------------------------------
	// MamaTimerCallback
	public void onTimer(MamaTimer timer)
	{
		try {
			String elapsed = MdsOmUtil.getElapsed();
			double divider = 1000000.0;
	
			Runtime r = Runtime.getRuntime();
			long totalMem = r.totalMemory() / (1024*1024);		// mb
			long freeMem = r.freeMemory() / (1024*1024);		// mb
			long maxMem = r.maxMemory() / (1024*1024);			// mb
			long usedMem = totalMem - freeMem;
	
			Mama.log(MamaLogLevel.NORMAL, "");
			Mama.log(MamaLogLevel.NORMAL, "STATS: process mem=" + usedMem + "/" + totalMem + "/" + maxMem + " / elapsed=" + elapsed);
	
			for (MdsOmEnv e : envs) {
	
				// Message rate
				long rate = e.countSubMsgsPeriod / statsInterval;
				e.countSubMsgsPeriod = 0;
				if (rate > e.maxSubMsgRate) e.maxSubMsgRate = rate;
				
				long pubRate = e.countPubMsgsPeriod / statsInterval;
				e.countPubMsgsPeriod = 0;
				if (pubRate > e.maxPubMsgRate) e.maxPubMsgRate = pubRate;
				
				double inOnMsgAvg = e.inOnMsgSum / (e.inOnMsgCount == 0 ? 1 : e.inOnMsgCount);
				double btwnOnMsgAvg = e.btwnOnMsgSum / (e.btwnOnMsgCount == 0 ? 1 : e.btwnOnMsgCount);
	
				if (inOnMsgAvg > e.maxInOnMsgAvg) e.maxInOnMsgAvg = (long) inOnMsgAvg;
				if (btwnOnMsgAvg > e.maxBtwnOnMsgAvg) e.maxBtwnOnMsgAvg = (long) btwnOnMsgAvg;
	
				e.inOnMsgSum = 0;
				e.inOnMsgCount = 0;
				e.btwnOnMsgSum = 0;
				e.btwnOnMsgCount = 0;
	
				String s = String.format("STATS: %s s=%d/%d/%d p=%d/%d/%d in=%.2f/%.2f btwn=%.2f/%.2f q=%d/%d",
					e.getName(),
					e.countSubs, rate, e.maxSubMsgRate, 
					e.countPubs, pubRate, e.maxPubMsgRate,
					inOnMsgAvg / divider, e.maxInOnMsgAvg / divider,
					btwnOnMsgAvg / divider, e.maxBtwnOnMsgAvg / divider,
					e.getQueue().getEventCount(), e.maxQueueCount);
				Mama.log(MamaLogLevel.NORMAL, s);
	
				s = String.format("STATS:      p=%d i=%d u=%d q=%d r=%d rec=%d np=%d nf=%d / bad=%d tm=%d stale=%d ne=%d np=%d nf=%d",
					e.countPubMsgs,
					e.countMAMA_MSG_TYPE_INITIAL, e.countMAMA_MSG_TYPE_UPDATE, e.countMAMA_MSG_TYPE_QUOTE, e.countResponses,
					e.countMAMA_MSG_TYPE_RECAP, e.countMAMA_MSG_TYPE_NOT_PERMISSIONED, e.countMAMA_MSG_TYPE_NOT_FOUND,
					e.countMAMA_MSG_STATUS_BAD_SYMBOL, e.countMAMA_MSG_STATUS_TIMEOUT, e.countMAMA_MSG_STATUS_STALE,
					e.countMAMA_MSG_STATUS_NOT_ENTITLED, e.countMAMA_MSG_STATUS_NOT_PERMISSIONED, e.countMAMA_MSG_STATUS_NOT_FOUND);
				Mama.log(MamaLogLevel.NORMAL, s);
			}
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.stats: error " + e.toString());
		}
	}
	
	public void onDestroy(MamaTimer timer)
	{
	}
} 

