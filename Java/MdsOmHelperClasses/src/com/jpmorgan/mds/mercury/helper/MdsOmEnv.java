package com.jpmorgan.mds.mercury.helper;

import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.wombat.mama.Mama;
import com.wombat.mama.MamaConnection;
import com.wombat.mama.MamaMsg;
import com.wombat.mama.MamaMsgStatus;
import com.wombat.mama.MamaMsgType;
import com.wombat.mama.MamaPayloadType;
import com.wombat.mama.MamaQueueGroup;
import com.wombat.mama.MamaSource;
import com.wombat.mama.MamaStatus;
import com.wombat.mama.MamaTransport;
import com.wombat.mama.MamaDictionary;
import com.wombat.mama.MamaSubscription;
import com.wombat.mama.MamaQueue;
import com.wombat.mama.MamaSubscriptionCallback;
import com.wombat.mama.MamaDictionaryCallback;
import com.wombat.mama.MamaTransportListener;
import com.wombat.mama.MamaLogLevel;
import com.wombat.mama.MamaBridge;
import com.wombat.mama.MamaFieldDescriptor;
import com.jpmorgan.mds.mercury.helper.MdsOmSubscription;
import com.jpmorgan.mds.mercury.helper.MdsOm.MdsOmEnvType;
import com.jpmorgan.mds.mercury.helper.MdsOmTransport.MdsOmTransportStatus;

public class MdsOmEnv implements MamaDictionaryCallback {
	private String name = null;
	
	// For Solace publish error callbacks
	private MdsOmEnvType type = MdsOmEnvType.UNKNOWN;
	
	
	private String bridgeName = null;
	private MamaBridge bridge = null;
	private MamaQueueGroup queueGroup = null;
	
	protected final Lock statsLock = new ReentrantLock();					// used to lock for stats
	
	private MdsOmConfig config;
	private MdsOmTransports transports = new MdsOmTransports();			// all of the transports for this env
	protected int countTransports = 0;
	
	private List<String> sourceEnvList = new ArrayList<String>();
	
	/** There is a single dictionary for the env */
	private MdsOmEnvStatus dictionaryStatus = MdsOmEnvStatus.WAIT;
	private MamaDictionary dictionary = new MamaDictionary();
	private Map<String, MamaFieldDescriptor> nameToFdMap = new HashMap<String, MamaFieldDescriptor>();
	private MamaFieldDescriptor[] fidToFdMap = null;
	
	/** OM payload id for this env/bridge */
	private char payloadId = MamaPayloadType.MAMA_PAYLOAD_UNKNOWN;
	
	/** Enable data quality as a subscriber */
	private boolean enableDq = true;
	
	/** Require an initial msg before accepting updates */
	private boolean requireInitial = true;
	
	// Stats vars ---------------------------------------------
	public boolean enableStats = true;
	public long countSubMsgs = 0;
	public long countSubMsgsPeriod = 0;
	public long countSubs = 0;
	public long countPubs = 0;
	public long countResponses = 0;
	public long countPubMsgs = 0;
	public long countPubMsgsPeriod = 0;
	
	public long maxSubMsgRate = 0;
	public long maxPubMsgRate = 0;
	public long maxQueueCount = 0;

	// Holds elapsed times for onMsg callback stats
	public long btwnOnMsgSum = 0;
	public long inOnMsgSum = 0;
	public long btwnOnMsgCount = 0;
	public long inOnMsgCount = 0;
	public long maxInOnMsgAvg = 0;
	public long maxBtwnOnMsgAvg = 0;
	
	// All of the msg types for stats counting
	public long countMAMA_MSG_TYPE_UPDATE = 0;
	public long countMAMA_MSG_TYPE_INITIAL = 0;
	public long countMAMA_MSG_TYPE_CANCEL = 0;
	public long countMAMA_MSG_TYPE_ERROR = 0;
	public long countMAMA_MSG_TYPE_CORRECTION = 0;
	public long countMAMA_MSG_TYPE_CLOSING = 0;
	public long countMAMA_MSG_TYPE_RECAP = 0;
	public long countMAMA_MSG_TYPE_DELETE = 0;
	public long countMAMA_MSG_TYPE_EXPIRE = 0;
	public long countMAMA_MSG_TYPE_SNAPSHOT = 0;
	public long countMAMA_MSG_TYPE_PREOPENING = 0;
	public long countMAMA_MSG_TYPE_QUOTE = 0;
	public long countMAMA_MSG_TYPE_TRADE = 0;
	public long countMAMA_MSG_TYPE_ORDER = 0;
	public long countMAMA_MSG_TYPE_BOOK_INITIAL = 0;
	public long countMAMA_MSG_TYPE_BOOK_UPDATE = 0;
	public long countMAMA_MSG_TYPE_BOOK_CLEAR = 0;
	public long countMAMA_MSG_TYPE_BOOK_RECAP = 0;
	public long countMAMA_MSG_TYPE_BOOK_SNAPSHOT = 0;
	public long countMAMA_MSG_TYPE_NOT_PERMISSIONED = 0;
	public long countMAMA_MSG_TYPE_NOT_FOUND = 0;
	public long countMAMA_MSG_TYPE_END_OF_INITIALS = 0;
	public long countMAMA_MSG_TYPE_WOMBAT_REQUEST = 0;
	public long countMAMA_MSG_TYPE_WOMBAT_CALC = 0;
	public long countMAMA_MSG_TYPE_SEC_STATUS = 0;
	public long countMAMA_MSG_TYPE_DDICT_SNAPSHOT = 0;
	public long countMAMA_MSG_TYPE_MISC = 0;
	public long countMAMA_MSG_TYPE_TIBRV = 0;
	public long countMAMA_MSG_TYPE_FEATURE_SET = 0;
	public long countMAMA_MSG_TYPE_SYNC_REQUEST = 0;
	public long countMAMA_MSG_TYPE_REFRESH = 0;
	public long countMAMA_MSG_TYPE_WORLD_VIEW = 0;
	public long countMAMA_MSG_TYPE_NEWS_QUERY = 0;
	public long countMAMA_MSG_TYPE_NULL = 0;

	// All of the status types for stats counting
	public long countMAMA_MSG_STATUS_OK = 0;
	public long countMAMA_MSG_STATUS_LINE_DOWN = 0;
	public long countMAMA_MSG_STATUS_NO_SUBSCRIBERS = 0;
	public long countMAMA_MSG_STATUS_BAD_SYMBOL = 0;
	public long countMAMA_MSG_STATUS_EXPIRED = 0;
	public long countMAMA_MSG_STATUS_TIMEOUT = 0;
	public long countMAMA_MSG_STATUS_MISC = 0;
	public long countMAMA_MSG_STATUS_STALE = 0;
	public long countMAMA_MSG_STATUS_PLATFORM_STATUS = 0;
	public long countMAMA_MSG_STATUS_NOT_ENTITLED = 0;
	public long countMAMA_MSG_STATUS_NOT_FOUND = 0;
	public long countMAMA_MSG_STATUS_POSSIBLY_STALE = 0;
	public long countMAMA_MSG_STATUS_NOT_PERMISSIONED = 0;
	public long countMAMA_MSG_STATUS_TOPIC_CHANGE = 0;
	public long countMAMA_MSG_STATUS_BANDWIDTH_EXCEEDED = 0;
	public long countMAMA_MSG_STATUS_DUPLICATE = 0;
	public long countMAMA_MSG_STATUS_UNKNOWN = 0;

	public static enum MdsOmEnvStatus {
		WAIT,		// waiting for connection
		SUCCESS,	// succesful connection
		FAIL		// failed connection
	};

	public MdsOmEnv() {
		transports.setEnv(this);
	}

	public void init(MdsOmConfig cfg, boolean enableStatistics) throws MdsOmStatus {

		this.config = new MdsOmConfig(cfg);
		this.type = config.type;
		this.enableStats = enableStatistics;

		config.dictionarySource = "WOMBAT";		// RMDS/Firefly/Solace all use this source for dictionaries
		
		switch (type) {
		case MERCURY:
		case SOLACE:
			name = "idpb";
			bridgeName = "solace";
			enableDq = false;
			requireInitial = true;
			payloadId = MamaPayloadType.MAMA_PAYLOAD_SOLACE;
			break;
		case TREP:
			name = "trep";
			bridgeName = "tick42rmds";
			enableDq = false;
			requireInitial = false;
			payloadId = MamaPayloadType.MAMA_PAYLOAD_TICK42RMDS;
			break;
		case MAMACACHE:
		case WMW:
			name = "mama";
			bridgeName = "wmw";
			enableDq = true;
			requireInitial = true;
			payloadId = MamaPayloadType.MAMA_PAYLOAD_WOMBAT_MSG;
			break;
		default:
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.init: invalid env=" + type);
			throw new MdsOmStatus(MdsOmStatus.INVALID_ENV);
		}

		Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.init: name=" + name + " bridge=" + bridgeName + " dq=" + enableDq + " payload=" + payloadId);
		Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.init: config=" + config.toString());
	}

	/**
	 * Load this env's source list from the mama.properties
	 */
	public void loadSourceList() {
		// Get mama property name
		// mama.solace.transport.rmds_tport.sourceMap
		String tname = config.transportName;
		if (config.transportNames.size() > 0) {
			tname = config.transportNames.get(0);		// use just one of these multiple named transports for the map
		}
		String buf = "mama." + bridgeName + ".transport." + tname + ".sourceMap";
		Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.loadSourceMap: " + name + " " + buf);
		String sourceMap = Mama.getProperty(buf);
		if (sourceMap != null) {
			// Comma delimited list of sources
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.loadSourceMap: " + sourceMap);
			String[] sources = sourceMap.split("[, \t\r\n]");
			for (String source : sources) {
				if (!sourceEnvList.contains(source)) {
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.loadSourceMap: adding " + source);
					sourceEnvList.add(source);
				}
			}
		}
	}

	/**
	 * Get a MamaMsg to publish.
	 * This method handles setting the correct payload id for this publisher.
	 * The msg is owned by the caller, but can be reused for the life of the application.
	 * @return             MamaMsg.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public MamaMsg getMamaMsg() throws Exception
	{
		return MdsOmUtil.newMamaMsg(getPayloadId());
	}

	/**
	 * Destroy a MamaMsg.
	 * @param              msg the MamaMsg to destroy.
	 * @return             None.
	 * @throws             MamaStatus, MdsOmStatus
	 */
	public void destroyMamaMsg(MamaMsg msg) throws Exception
	{
		MdsOmUtil.deleteMamaMsg(msg);
	}
	
	public boolean findSourceList(String source) {
		for (String s : sourceEnvList) {
			// System.out.println(getName() + ": '" + source + "' '" + s + "'");
			if (source.equalsIgnoreCase(s)) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * Get a field descriptor from a fid.
	 * @param fid
	 * @return
	 */
	public MamaFieldDescriptor getFieldDescriptor(int fid)
	{
		MamaFieldDescriptor fieldDescriptor = null;
		if (getDictionary() == null) {
			// No dictionary
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldDescriptor: " + getName() + " no dictionary");
			return null;
		}
		if (fidToFdMap != null) {
			// This is a map for faster access
			fieldDescriptor = fidToFdMap[fid];
			if (fieldDescriptor == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldFid: " + getName() + " cannot find field '" + fid + "'");
				return null;				
			}
		} else {
			fieldDescriptor = getDictionary().getFieldByFid(fid);
			if (fieldDescriptor == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldDescriptor: " + getName() + "  cannot find field '" + fid + "'");
				return null;
			}
		}
		return fieldDescriptor;
	}

	/**
	 * Get a field description from a field name.
	 * @param fieldName
	 * @return
	 */
	public MamaFieldDescriptor getFieldDescriptor(String fieldName)
	{
		MamaFieldDescriptor fieldDescriptor = null;
		if (fieldName == null) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldDescriptor: " + getName() + " cannot find field with NULL field name");
			return null;
		}
		if (nameToFdMap.size() > 0) {
			// This is a map for faster access
			fieldDescriptor = nameToFdMap.get(fieldName);
			if (fieldDescriptor == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldFid: " + getName() + " cannot find field '" + fieldName + "'");
				return null;				
			}
		} else {
			if (getDictionary() == null) {
				// No dictionary
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldDescriptor: " + getName() + "  no dictionary");
				return null;
			}
			// This is a serial search using strcmp, so it will be slow - we have 30k fields in the dictionary
			fieldDescriptor = getDictionary().getFieldByName(fieldName);
			if (fieldDescriptor == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldDescriptor: " + getName() + "  cannot find field '" + fieldName + "'");
				return null;
			}
		}
		return fieldDescriptor;
	}

	/**
	 * Get a field name from a field fid.
	 * @param fid
	 * @return
	 */
	public String getFieldName(int fid)
	{
		MamaFieldDescriptor fieldDescriptor = null;
		if (fidToFdMap != null) {
			// This is a map for faster access
			fieldDescriptor = fidToFdMap[fid];
			if (fieldDescriptor == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldFid: " + getName() + " cannot find field '" + fid + "'");
				return null;				
			}
		} else {
			if (getDictionary() == null) {
				// No dictionary
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldName: " + getName() + " no dictionary");
				return null;
			}
			fieldDescriptor = getDictionary().getFieldByFid(fid);
			if (fieldDescriptor == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldDescriptor: " + getName() + "  cannot find field '" + fid + "'");
				return null;
			}
		}
		return fieldDescriptor.getName();
	}

	/**
	 * Get a field fid from a field name.
	 * @param fieldName
	 * @return
	 */
	public int getFieldFid(String fieldName)
	{
		if (fieldName == null) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldFid: " + getName() + " cannot find field with NULL field name");
			return 0;
		}
		if (nameToFdMap.size() > 0) {
			// This is a map for faster access
			MamaFieldDescriptor fieldDescriptord = nameToFdMap.get(fieldName);
			if (fieldDescriptord == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldFid: " + getName() + " cannot find field '" + fieldName + "'");
				return 0;				
			}
			return fieldDescriptord.getFid();
		} else {
			if (getDictionary() == null) {
				// No dictionary
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldFid: " + getName() + " no dictionary");
				return 0;
			}
			// This is a serial search using strcmp, so it will be slow - we have 30k fields in the dictionary
			MamaFieldDescriptor fieldDescriptor = getDictionary().getFieldByName(fieldName);
			if (fieldDescriptor == null) {
				// Cannot find this field
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getFieldFid: " + getName() + " cannot find field '" + fieldName + "'");
				return 0;
			}
			return fieldDescriptor.getFid();
		}
	}

	/**
	 * Write the dictionary to a file.
	 * Mostly used for debugging.
	 * @param fileName
	 * @return
	 */
	public boolean writeDictionary(String fileName)
	{
		if (fileName == null) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::writeDictionary: " + name + "null file name");
			return false;
		}
		if (getDictionary() == null) {
			// No dictionary
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv::writeDictionary: " + getName() + " no dictionary");
			return false;
		}
		MamaDictionary d = getDictionary();
		d.writeToFile(fileName);
		return true;
	}

	/**
	 * Load the environment and the bridge.
	 */
	public void load() {
		try {
			bridge = Mama.loadBridge(bridgeName);
		} catch (Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.load: error " + getName() + " " + bridgeName + " " + MdsOmUtil.getStackTrace(e));
		}
	}

	/**
	 * Allows mama.properties values to be altered
	 */
	public void setProperty(String property, String value){
		Mama.setProperty(property, value);
	}
	
	/**
	 * Determines if value exists in mama.properties file
	 */
	public boolean propertyExists(String property){
		if(Mama.getProperty(property) != null)
			return true;
		else
			return false;
	}
	
	/**
	 * Determines if user has been defined in mama.properties file
	 */
	public boolean userExists(MdsOmConfig cfg) {
		switch(cfg.type) {
		case MAMACACHE:
		case WMW:
			break;
		case MERCURY:
		case SOLACE:
			return propertyExists("mama."+bridgeName+".transport."+cfg.transportName+".session_username");
		case TREP:
			return propertyExists("mama."+bridgeName+".transport."+cfg.transportName+".user");
		case UNKNOWN:
		default:
			break;
		}
		return false;
	}
	
	/**
	 * Overrides/Sets user name defined in mama.properties
	 */
	public void setUser(MdsOmConfig cfg) {
		if (cfg.username != null && !cfg.username.isEmpty()) {
			switch(cfg.type) {
			case MAMACACHE:
			case WMW:
				break;
			case MERCURY:
			case SOLACE:
				setProperty("mama."+bridgeName+".transport."+cfg.transportName+".session_username",cfg.username);
				break;
			case TREP:
				setProperty("mama."+bridgeName+".transport."+cfg.transportName+".user",cfg.username);
				break;
			case UNKNOWN:
				break;
			default:
				break;
			}
		}
	}
	
	/**
	 * Open the env and the bridge.
	 * This opens the transport to the infrastructure.
	 * @throws MdsOmStatus
	 */
	public void open() throws MdsOmStatus {
		try {
			if (config.transportName.isEmpty() && config.transportNames.size() == 0) {
				// They have to give us a transport name
				Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.open: " + name
						+ " no transport name provided");
				throw new MdsOmStatus(MdsOmStatus.TRANSPORT_NOT_PROVIDED);
			}

			loadSourceList();
	
			// Check for multiple transports
			// If the lists are empty then it will default to a single transport
			config.transportListIndex = 0;
			if (config.transportNames.size() > 0) {
				config.roundRobinTransportCount = config.transportNames.size();
				Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + name
						+ " create named transports="
						+ config.roundRobinTransportCount);
				
				queueGroup = getQueueGroup(config.roundRobinTransportCount, bridge);
				
				for (int i = 0; i < config.roundRobinTransportCount; ++i) {
					String buf = name + "-" + (i + 1);
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + name + " create named transport " + buf + " " + config.transportNames.get(i));
					countTransports++;
					MamaTransport transport = getTransport(buf);
					transports.addTransport(transport);
					if(!userExists(config))
						setUser(config);
						
					createTransport(transport, config.transportNames.get(i));
				}
			} else if (config.roundRobinTransportCount > 0) {
				Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + name
						+ " create round robin transports="
						+ config.roundRobinTransportCount);
				
				queueGroup = getQueueGroup(config.roundRobinTransportCount, bridge);
				
				for (int i = 0; i < config.roundRobinTransportCount; ++i) {
					String buf = name + "-" + (i + 1);
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + name + " create round robin transport " + buf);
					countTransports++;
					MamaTransport transport = getTransport(buf);
					transports.addTransport(transport);
					if(!userExists(config))
						setUser(config);
					
					createTransport(transport, config.transportName);
				}
			} else {
				Mama.log(MamaLogLevel.NORMAL,
						"MdsOmEnv::open: " + name
								+ " create source transports="
								+ config.sourceMap.size());
				for (Map.Entry<String, List<String>> entry : config.sourceMap.entrySet()) {
					String transportName = entry.getKey();
					List<String> sources = entry.getValue();
					if (sources.size() == 0) {
						Mama.log(MamaLogLevel.WARN, "MdsOmEnv.open: " + transportName
								+ " found list of " + name + " sources with 0 names");
						continue;
					}
					System.out.println("source size=" + sources.size());

					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + transportName + " create source transport " + name);
					countTransports++;
					MamaTransport transport = getTransport("ALL");

					// For each source on the transport add it to the map
					for (String source : sources) {
						transports.addTransport(transport, source);
						Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + name + "     add source " + source);
					}
					if(!userExists(config))
						setUser(config);

					createTransport(transport, transport.getName());
				}
				if (transports.isMap()) {
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + name + " create queue group size=" + transports.mapSize());
					queueGroup = getQueueGroup(transports.mapSize(), bridge);
				} else {
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.open: " + name + " create single transport ALL");
					queueGroup = getQueueGroup(1, bridge);
					countTransports++;
					MamaTransport transport = getTransport("All");
					transports.addTransport(transport);
					if(!userExists(config))
						setUser(config);

					createTransport(transport, config.transportName);
				}
			}
		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.open: error " + getName() + " " + bridgeName + " " + MdsOmUtil.getStackTrace(status));
			throw status;
		}
	}

	public void printQueueGroup(MamaQueueGroup qg) {
		for(int i = 0; i < qg.getNumberOfQueues(); i++) {
			MamaQueue q = qg.getNextQueue();
			Mama.log(MamaLogLevel.NORMAL,  "MdsOmEnv.printQueueGroup: #" + (i+1) + " count=" + q.getEventCount());
		}
	}
	
	private MamaQueueGroup getQueueGroup(int size, MamaBridge bridge) {
		MamaQueueGroup queueGroup = new MamaQueueGroup(size, bridge);

		for (int i = 0; i < queueGroup.getNumberOfQueues(); i++) {
			// MamaQueue q = queueGroup.getNextQueue();
			// The C++ version uses the closure on the MamaQueue to store a timestamp for timing, but that isn't available on the Java MamaQueue
		}

		printQueueGroup(queueGroup);

		return queueGroup;
	}

	/**
	 * Get a transport, but do not open/create it.
	 * @param descr
	 * @return
	 */
	public MamaTransport getTransport(String descr) {	
		MamaTransport transport = new MamaTransport();
		transport.addTransportListener(new MdsOmEnvTransportListener(this, transport));
		transport.setDescription(descr);
		return transport;
	}

	/**
	 * Create/open a transport.
	 * This connects to the infrastructure.
	 * @param transport
	 * @param name
	 */
	public void createTransport(MamaTransport transport, String name)
	{
		transport.create(name, bridge);
	}
	
	/**
	 * Called during shutdown to stop the MamaQueues.
	 * @throws Exception
	 */
	public void stopQueues() throws Exception {
		try {
			if (dictionary != null) {
				Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.stopQueues: " + name + " delete dictionary");
				dictionary = null;
			}
			
			if (queueGroup != null) {
				Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.stopQueues: " + name + " group=" + queueGroup);
				printQueueGroup(queueGroup);
				
				try {
					// TODO this creates an error since the queue has interest count > 0
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.stopQueues: " + name + " call destroyWait");
					queueGroup.destroyWait();
				} catch(Exception e) {
					Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.stopQueues: TEMPORARY error " + name + " " + e.toString());
				}
				queueGroup = null;
			}
		} catch(Exception e) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.stopQueues: error " + name + " " + e.toString());
			//send original exception reference back to caller
			throw e;
		}
	}
	
	/**
	 * Close this env.
	 * @throws Exception
	 */
	public void close() throws Exception {
		transports.close();
	}
	
	public void setDictionaryFile(String fileName) throws MdsOmStatus {
		if (fileName == null) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::setDictionaryFile: null dictionary file name passed in " + name);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		config.dictionaryFile = fileName;
	}

	// SNAP ---------------------------------------------------------------------------------------
	public MdsOmSubscription snap(String topic,
			MamaSubscriptionCallback cb, Object closure, boolean setup)
			throws MdsOmStatus
	{
		if (topic == null || cb == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm::subscribe: null arg passed in cb=" + cb + " topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return snap(s.getX(), s.getY(), cb, closure, setup);
	}

	public MdsOmSubscription snap(String source, String symbol,
			MamaSubscriptionCallback cb, Object closure, boolean setup)
			throws MdsOmStatus
	{
		if (Mama.getLogLevel().intValue() >= MamaLogLevel.FINE.getValue())
			Mama.log(MamaLogLevel.FINE, "MdsOmEnv.subscribe: " + name + " cb=" + cb + " source=" + source + " symbol=" + symbol);
		
		if (source == null || symbol == null || cb == null) {
			Mama.log(MamaLogLevel.ERROR,"MdsOmEnv.subscribe: null arg passed in cb=" + cb + " source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		source = source.trim();
		symbol = symbol.trim();
		
		if (source.length() == 0 || symbol.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOmEnv.subscribe: empty arg passed in cb=" + cb + " source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
			
		try {
			countSubs++;

			MdsOmSubscription sub = new MdsOmSubscription();
			sub.snap(this, transports.getTransport(source), source, symbol, cb, closure, setup);

			return sub;
		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.subscribe: error " + getName() + " " + bridgeName + " " + MdsOmUtil.getStackTrace(status));
			throw status;
		}
	}

	// SUBSCRIBE ---------------------------------------------------------------------------------------
	public MdsOmSubscription subscribe(String topic,
			MamaSubscriptionCallback cb, Object closure, boolean setup)
			throws MdsOmStatus
	{
		if (topic == null || cb == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOm::subscribe: null arg passed in cb=" + cb + " topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return subscribe(s.getX(), s.getY(), cb, closure, setup);
	}

	public MdsOmSubscription subscribe(String source, String symbol,
			MamaSubscriptionCallback cb, Object closure, boolean setup)
			throws MdsOmStatus
	{
		if (Mama.getLogLevel().intValue() >= MamaLogLevel.FINE.getValue())
			Mama.log(MamaLogLevel.FINE, "MdsOmEnv.subscribe: " + name + " cb=" + cb + " source=" + source + " symbol=" + symbol);
		
		if (source == null || symbol == null || cb == null) {
			Mama.log(MamaLogLevel.ERROR,"MdsOmEnv.subscribe: null arg passed in cb=" + cb + " source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		source = source.trim();
		symbol = symbol.trim();
		
		if (source.length() == 0 || symbol.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,"MdsOmEnv.subscribe: empty arg passed in cb=" + cb + " source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}
			
		try {
			countSubs++;

			MdsOmSubscription sub = new MdsOmSubscription();
			sub.create(this, transports.getTransport(source), source, symbol, cb, closure, setup);

			return sub;
		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.subscribe: error " + getName() + " " + bridgeName + " " + MdsOmUtil.getStackTrace(status));
			throw status;
		}
	}

	// PUBLISHER -------------------------------------------------------------------------------------------------------------
	public MdsOmPublisher getPublisher(String topic, MdsOmPublisherCallback cb, Object closure) throws MdsOmStatus {
		if (topic == null || topic.length() == 0) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv::getPublisher: " + name
					+ " null arg passed in topic=" + topic);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		Pair<String, String> s = MdsOmUtil.getSourceAndSymbol(topic);
		return getPublisher(s.getX(), s.getY(), cb, closure);
	}

	public MdsOmPublisher getPublisher(String source, String symbol, MdsOmPublisherCallback cb, Object closure)
			throws MdsOmStatus {
		if (source == null || symbol == null || source.length() == 0 || symbol.length() == 0) {
			Mama.log(MamaLogLevel.ERROR,
					"MdsOmEnv::getPublisher: null arg passed in " + name
							+ " source=" + source + " symbol=" + symbol);
			throw new MdsOmStatus(MdsOmStatus.NULL_ARG);
		}

		countPubs++;

		MdsOmPublisher pub = new MdsOmPublisher();
		pub.create(this, transports.getTransport(source), source, symbol, cb, closure);
		return pub;
	}

	public void getTransports(List<MamaTransport> transportList)
	{
		transports.getTransports(transportList);
	}

	public void loadDictionary() throws InterruptedException, MdsOmStatus {
		try {
			if (!config.dictionaryFile.isEmpty()) {
				Mama.log(MamaLogLevel.NORMAL,
						"MdsOmEnv.loadDictionary: load dictionary for " + name + " from " + config.dictionaryFile);
				dictionary.populateFromFile(config.dictionaryFile);
				dictionaryStatus = MdsOmEnvStatus.SUCCESS;
			} else if (!config.dictionarySource.isEmpty()) {
				MamaTransport t = transports.getTransport();
				
				Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.loadDictionary: load dictionary for " + name + " from " + config.dictionarySource + " on " + t.getName());
			
				// NOTE: the Java OM interface to get a dictionary is different from the C++ OM.
				MamaSource sourceDictionary = new MamaSource(config.dictionarySource, t, config.dictionarySource);
				MamaSubscription subscription = new MamaSubscription();
				dictionary = subscription.createDictionarySubscription(this, queueGroup.getNextQueue(), sourceDictionary, 30.0, 3);

				Mama.start(bridge);
				
				// Create name-to-fid map for faster access
				if (dictionary != null && dictionaryStatus == MdsOmEnvStatus.SUCCESS) {
					// Get array to max fid, any with no fid will be null
					fidToFdMap = new MamaFieldDescriptor[dictionary.getMaxFid() + 1];
					for (int i = 0; i < dictionary.getSize(); ++i) {
						MamaFieldDescriptor fd = null;
						try {
							fd = dictionary.getFieldByIndex(i);
						} catch (Exception ex) {
							// A bridge (wmw) may throw an exception if there is no fid at an index
						}
						if (fd != null) {
							nameToFdMap.put(fd.getName(), fd);
							fidToFdMap[fd.getFid()] = fd;
						}
					}
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.loadDictionary: created name map size=" + nameToFdMap.size());
				}
			} else {
				Mama.log(MamaLogLevel.ERROR,
						"MdsOmEnv.loadDictionary: load dictionary for " + name + " from nowhere");
				throw new MdsOmStatus(MdsOmStatus.DICTIONARY_NO_SOURCE);
			}
		} catch (MdsOmStatus status) {
			Mama.log(MamaLogLevel.ERROR, "MdsOmEnv.loadDictionary: error " + getName() + " " + bridgeName + " " + MdsOmUtil.getStackTrace(status));
			throw status;
		}
	}

	// ----------------------------------------------------------
	// MamaDictionaryCallback
	public void onTimeout() {
		Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onTimeout: timed out waiting for dictionary " + name);
		dictionaryStatus = MdsOmEnvStatus.FAIL;
		Mama.stop(bridge);
	}

	public void onError(String errMsg) {
		Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onError: error for dictionary " + name + " error=" + errMsg);
		dictionaryStatus = MdsOmEnvStatus.FAIL;
		Mama.stop(bridge);
	}

	public void onComplete() {
		Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onComplete: Got dictionary "
				+ name + " size=" + dictionary.getSize() + " maxFid="
				+ dictionary.getMaxFid());
		dictionaryStatus = MdsOmEnvStatus.SUCCESS;
		Mama.stop(bridge);
	}
	
	// ----------------------------------------------------------
	public void countTypes(short type, int status) {
		switch (type) {
		case MamaMsgType.TYPE_UPDATE:
			countMAMA_MSG_TYPE_UPDATE++;
			break;
		case MamaMsgType.TYPE_INITIAL:
			countMAMA_MSG_TYPE_INITIAL++;
			break;
		case MamaMsgType.TYPE_CANCEL:
			countMAMA_MSG_TYPE_CANCEL++;
			break;
		case MamaMsgType.TYPE_ERROR:
			countMAMA_MSG_TYPE_ERROR++;
			break;
		case MamaMsgType.TYPE_CORRECTION:
			countMAMA_MSG_TYPE_CORRECTION++;
			break;
		case MamaMsgType.TYPE_CLOSING:
			countMAMA_MSG_TYPE_CLOSING++;
			break;
		case MamaMsgType.TYPE_RECAP:
			countMAMA_MSG_TYPE_RECAP++;
			break;
		case MamaMsgType.TYPE_DELETE:
			countMAMA_MSG_TYPE_DELETE++;
			break;
		case MamaMsgType.TYPE_EXPIRE:
			countMAMA_MSG_TYPE_EXPIRE++;
			break;
		case MamaMsgType.TYPE_SNAPSHOT:
			countMAMA_MSG_TYPE_SNAPSHOT++;
			break;
		case MamaMsgType.TYPE_PREOPENING:
			countMAMA_MSG_TYPE_PREOPENING++;
			break;
		case MamaMsgType.TYPE_QUOTE:
			countMAMA_MSG_TYPE_QUOTE++;
			break;
		case MamaMsgType.TYPE_TRADE:
			countMAMA_MSG_TYPE_TRADE++;
			break;
		case MamaMsgType.TYPE_ORDER:
			countMAMA_MSG_TYPE_ORDER++;
			break;
		case MamaMsgType.TYPE_BOOK_INITIAL:
			countMAMA_MSG_TYPE_BOOK_INITIAL++;
			break;
		case MamaMsgType.TYPE_BOOK_UPDATE:
			countMAMA_MSG_TYPE_BOOK_UPDATE++;
			break;
		case MamaMsgType.TYPE_BOOK_CLEAR:
			countMAMA_MSG_TYPE_BOOK_CLEAR++;
			break;
		case MamaMsgType.TYPE_BOOK_RECAP:
			countMAMA_MSG_TYPE_BOOK_RECAP++;
			break;
		case MamaMsgType.TYPE_BOOK_SNAPSHOT:
			countMAMA_MSG_TYPE_BOOK_SNAPSHOT++;
			break;
		case MamaMsgType.TYPE_NOT_PERMISSIONED:
			countMAMA_MSG_TYPE_NOT_PERMISSIONED++;
			break;
		case MamaMsgType.TYPE_NOT_FOUND:
			countMAMA_MSG_TYPE_NOT_FOUND++;
			break;
		case MamaMsgType.TYPE_END_OF_INITIALS:
			countMAMA_MSG_TYPE_END_OF_INITIALS++;
			break;
		case MamaMsgType.TYPE_WOMBAT_REQUEST:
			countMAMA_MSG_TYPE_WOMBAT_REQUEST++;
			break;
		case MamaMsgType.TYPE_WOMBAT_CALC:
			countMAMA_MSG_TYPE_WOMBAT_CALC++;
			break;
		case MamaMsgType.TYPE_SEC_STATUS:
			countMAMA_MSG_TYPE_SEC_STATUS++;
			break;
		case MamaMsgType.TYPE_DDICT_SNAPSHOT:
			countMAMA_MSG_TYPE_DDICT_SNAPSHOT++;
			break;
		case MamaMsgType.TYPE_MISC:
			countMAMA_MSG_TYPE_MISC++;
			break;
		case MamaMsgType.TYPE_PLATFORM:
			countMAMA_MSG_TYPE_TIBRV++;
			break;
		case MamaMsgType.TYPE_FEATURE_SET:
			countMAMA_MSG_TYPE_FEATURE_SET++;
			break;
		case MamaMsgType.TYPE_SYNC_REQUEST:
			countMAMA_MSG_TYPE_SYNC_REQUEST++;
			break;
		case MamaMsgType.TYPE_REFRESH:
			countMAMA_MSG_TYPE_REFRESH++;
			break;
		case MamaMsgType.TYPE_NULL:
			countMAMA_MSG_TYPE_NULL++;
			break;
		}

		switch (status) {
		case MamaMsgStatus.STATUS_OK:
			countMAMA_MSG_STATUS_OK++;
			break;
		case MamaMsgStatus.STATUS_LINE_DOWN:
			countMAMA_MSG_STATUS_LINE_DOWN++;
			break;
		case MamaMsgStatus.STATUS_NO_SUBSCRIBERS:
			countMAMA_MSG_STATUS_NO_SUBSCRIBERS++;
			break;
		case MamaMsgStatus.STATUS_BAD_SYMBOL:
			countMAMA_MSG_STATUS_BAD_SYMBOL++;
			break;
		case MamaMsgStatus.STATUS_EXPIRED:
			countMAMA_MSG_STATUS_EXPIRED++;
			break;
		case MamaMsgStatus.STATUS_TIMEOUT:
			countMAMA_MSG_STATUS_TIMEOUT++;
			break;
		case MamaMsgStatus.STATUS_MISC:
			countMAMA_MSG_STATUS_MISC++;
			break;
		case MamaMsgStatus.STATUS_STALE:
			countMAMA_MSG_STATUS_STALE++;
			break;
		case MamaMsgStatus.STATUS_PLATFORM_STATUS:
			countMAMA_MSG_STATUS_PLATFORM_STATUS++;
			break;
		case MamaMsgStatus.STATUS_NOT_ENTITLED:
			countMAMA_MSG_STATUS_NOT_ENTITLED++;
			break;
		case MamaMsgStatus.STATUS_NOT_FOUND:
			countMAMA_MSG_STATUS_NOT_FOUND++;
			break;
		case MamaMsgStatus.STATUS_POSSIBLY_STALE:
			countMAMA_MSG_STATUS_POSSIBLY_STALE++;
			break;
		case MamaMsgStatus.STATUS_NOT_PERMISSIONED:
			countMAMA_MSG_STATUS_NOT_PERMISSIONED++;
			break;
		case MamaMsgStatus.STATUS_TOPIC_CHANGE:
			countMAMA_MSG_STATUS_TOPIC_CHANGE++;
			break;
		case MamaMsgStatus.STATUS_BANDWIDTH_EXCEEDED:
			countMAMA_MSG_STATUS_BANDWIDTH_EXCEEDED++;
			break;
		case MamaMsgStatus.STATUS_DUPLICATE:
			countMAMA_MSG_STATUS_DUPLICATE++;
			break;
		default:
			countMAMA_MSG_STATUS_UNKNOWN++;
			break;
		}
	}

	public MdsOmEnvType getType() {
		return type;
	}

	public boolean isIdp() {
		return type == MdsOmEnvType.MERCURY || type == MdsOmEnvType.SOLACE;
	}

	public boolean isRmds() {
		return type == MdsOmEnvType.TREP;
	}

	public boolean isFirefly() {
		return type == MdsOmEnvType.MAMACACHE || type == MdsOmEnvType.WMW;
	}

	public boolean isConnected() {
		return transports.isConnected();
	}

	public boolean isConnectWait() {
		return transports.isConnectWait();
	}

	public boolean isConnectFail() {
		return transports.isConnectFail();
	}

	public boolean isDictionaryOk() {
		return dictionaryStatus == MdsOmEnvStatus.SUCCESS;
	}

	public boolean isDictionaryWait() {
		return dictionaryStatus == MdsOmEnvStatus.WAIT;
	}

	public boolean isDictionaryFail() {
		return dictionaryStatus == MdsOmEnvStatus.FAIL;
	}

	public boolean isEnableDq() {
		return enableDq;
	}

	public void setEnableDq(boolean enableDq) {
		this.enableDq = enableDq;
	}

	public MamaBridge getBridge() {
		return bridge;
	}

	public MamaQueue getQueue() {
		return queueGroup.getNextQueue();
	}

	public MamaDictionary getDictionary() {
		return dictionary;
	}

	public String getBridgeName() {
		return bridgeName;
	}

	public void setBridgeName(String bridgeName) {
		this.bridgeName = bridgeName;
	}

	public String getTransportName() {
		return config.transportName;
	}

	public void setTransportName(String transportName) {
		config.transportName = transportName;
	}

	public String getName() {
		return name;
	}

	public char getPayloadId() {
		return payloadId;
	}

	public boolean isRequireInitial() {
		return requireInitial;
	}

	public void setRequireInitial(boolean requireInitial) {
		this.requireInitial = requireInitial;
	}
	
	// ----------------------------------------------------------
	// MamaTransportCallback
	// Need this because Java OM does not return the transport with the callback
	class MdsOmEnvTransportListener implements MamaTransportListener {
		private MamaTransport transport;
		private MdsOmEnv env;

		public MdsOmEnv getEnv() {
			return env;
		}

		public void setEnv(MdsOmEnv env) {
			this.env = env;
		}

		public MamaTransport getTransport() {
			return transport;
		}

		public void setTransport(MamaTransport transport) {
			this.transport = transport;
		}
	
		public MdsOmEnvTransportListener(MdsOmEnv env, MamaTransport transport)
		{
			this.env = env;
			this.transport = transport;
		}
	
		public void logPlatformInfo(MamaTransport t, String msg, Object platformInfo) {
			// For now don't try to print this
			if (t != null && platformInfo != null) {
				String tname = t.getName();
				if (tname != null && tname.equals("wmw")) {
					MamaConnection p = (MamaConnection) platformInfo;
					Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv." + msg + " ip=" + p.getIpAddress() + ":" + p.getPort() + " user=" + p.getUserName() + " source=" + p.getAppName());
				}
			}
		}
	
		@Override
		public void onDisconnect(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onDisconnect: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			transports.setStatus(transport, MdsOmTransportStatus.FAIL);
			logPlatformInfo(transport, "onDisconnect", platformInfo);
			if (config.transportCb != null) config.transportCb.onDisconnect(env, transport, platformInfo);
		}
	
		@Override
		public void onReconnect(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onReconnect: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			transports.setStatus(transport, MdsOmTransportStatus.SUCCESS);
			logPlatformInfo(transport, "onReconnect", platformInfo);
			if (config.transportCb != null) config.transportCb.onReconnect(env, transport, platformInfo);
		}
	
		@Override
		public void onQuality(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onQuality: " + env.getName() + " " + transport.getName() + " cause=" + cause + " q=" + transport.getQuality());
			logPlatformInfo(transport, "onQuality", platformInfo);
			if (config.transportCb != null) config.transportCb.onQuality(env, transport, transport.getQuality(), platformInfo);
		}
	
		@Override
		public void onConnect(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onConnect: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			transports.setStatus(transport, MdsOmTransportStatus.SUCCESS);
			logPlatformInfo(transport, "onConnect", platformInfo);
			if (config.transportCb != null) config.transportCb.onConnect(env, transport, platformInfo);
		}
	
		@Override
		public void onAccept(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onAccept: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			transports.setStatus(transport, MdsOmTransportStatus.SUCCESS);
			logPlatformInfo(transport, "onAccept", platformInfo);
			if (config.transportCb != null) config.transportCb.onAccept(env, transport, platformInfo);
		}
	
		@Override
		public void onAcceptReconnect(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onAcceptReconnect: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			transports.setStatus(transport, MdsOmTransportStatus.SUCCESS);
			logPlatformInfo(transport, "onAcceptReconnect", platformInfo);
			if (config.transportCb != null) config.transportCb.onAcceptReconnect(env, transport, platformInfo);
		}
	
		@Override
		public void onPublisherDisconnect(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onPublisherDisconnect: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			logPlatformInfo(transport, "onPublisherDisconnect", platformInfo);
			if (config.transportCb != null) config.transportCb.onPublisherDisconnect(env, transport, platformInfo);
		}
	
		@Override
		public void onNamingServiceConnect(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onNamingServiceConnect: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			logPlatformInfo(transport, "onNamingServiceConnect", platformInfo);
			if (config.transportCb != null) config.transportCb.onNamingServiceConnect(env, transport, platformInfo);

		}
	
		@Override
		public void onNamingServiceDisconnect(short cause, Object platformInfo) {
			Mama.log(MamaLogLevel.NORMAL, "MdsOmEnv.onNamingServiceDisconnect: " + env.getName() + " " + transport.getName() + " cause=" + cause);
			logPlatformInfo(transport, "onNamingServiceDisconnect", platformInfo);
			if (config.transportCb != null) config.transportCb.onNamingServiceDisconnect(env, transport, platformInfo);
		}
	}
}
