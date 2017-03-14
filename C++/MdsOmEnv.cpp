
#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

MdsOmEnv::MdsOmEnv()
{
	type = MDS_OM_ENV_UNKNOWN;

	name = NULL;

	bridgeName = NULL;
	bridge = NULL;
	queueGroup = NULL;

	dictionary = NULL;

	payloadId = MAMA_PAYLOAD_UNKNOWN;

	enableDq = true;
	requireInitial = true;
	handlesDefaultSource = false;

	dictionaryStatus = MDS_OM_ENV_STATUS_WAIT;

	transports.setEnv(this);

	btwnOnMsgSum = 0;
	inOnMsgSum = 0;
	btwnOnMsgCount = 0;
	inOnMsgCount = 0;
	maxInOnMsgAvg = 0;
	maxBtwnOnMsgAvg = 0;
	countTransport = 0;

	maxSubMsgRate = 0;
	maxPubMsgRate = 0;
	maxQueueCount = 0;

	countSubMsgs = 0;
	countSubMsgsPeriod = 0;
	countSubs = 0;
	countPubs = 0;
	countResponses = 0;
	countPubMsgs = 0;
	countPubMsgsPeriod = 0;

	enableStats = true;
	wthread_mutex_init(&statsLock, NULL);

	countMAMA_MSG_TYPE_UPDATE = 0;
	countMAMA_MSG_TYPE_INITIAL = 0;
	countMAMA_MSG_TYPE_CANCEL = 0;
	countMAMA_MSG_TYPE_ERROR = 0;
	countMAMA_MSG_TYPE_CORRECTION = 0;
	countMAMA_MSG_TYPE_CLOSING = 0;
	countMAMA_MSG_TYPE_RECAP = 0;
	countMAMA_MSG_TYPE_DELETE = 0;
	countMAMA_MSG_TYPE_EXPIRE = 0;
	countMAMA_MSG_TYPE_SNAPSHOT = 0;
	countMAMA_MSG_TYPE_PREOPENING = 0;
	countMAMA_MSG_TYPE_QUOTE = 0;
	countMAMA_MSG_TYPE_TRADE = 0;
	countMAMA_MSG_TYPE_ORDER = 0;
	countMAMA_MSG_TYPE_BOOK_INITIAL = 0;
	countMAMA_MSG_TYPE_BOOK_UPDATE = 0;
	countMAMA_MSG_TYPE_BOOK_CLEAR = 0;
	countMAMA_MSG_TYPE_BOOK_RECAP = 0;
	countMAMA_MSG_TYPE_BOOK_SNAPSHOT = 0;
	countMAMA_MSG_TYPE_NOT_PERMISSIONED = 0;
	countMAMA_MSG_TYPE_NOT_FOUND = 0;
	countMAMA_MSG_TYPE_END_OF_INITIALS = 0;
	countMAMA_MSG_TYPE_WOMBAT_REQUEST = 0;
	countMAMA_MSG_TYPE_WOMBAT_CALC = 0;
	countMAMA_MSG_TYPE_SEC_STATUS = 0;
	countMAMA_MSG_TYPE_DDICT_SNAPSHOT = 0;
	countMAMA_MSG_TYPE_MISC = 0;
	countMAMA_MSG_TYPE_TIBRV = 0;
	countMAMA_MSG_TYPE_FEATURE_SET = 0;
	countMAMA_MSG_TYPE_SYNC_REQUEST = 0;
	countMAMA_MSG_TYPE_REFRESH = 0;
	countMAMA_MSG_TYPE_WORLD_VIEW = 0;
	countMAMA_MSG_TYPE_NEWS_QUERY = 0;
	countMAMA_MSG_TYPE_NULL = 0;

	countMAMA_MSG_STATUS_OK = 0;
	countMAMA_MSG_STATUS_LINE_DOWN = 0;
	countMAMA_MSG_STATUS_NO_SUBSCRIBERS = 0;
	countMAMA_MSG_STATUS_BAD_SYMBOL = 0;
	countMAMA_MSG_STATUS_EXPIRED = 0;
	countMAMA_MSG_STATUS_TIMEOUT = 0;
	countMAMA_MSG_STATUS_MISC = 0;
	countMAMA_MSG_STATUS_STALE = 0;
	countMAMA_MSG_STATUS_PLATFORM_STATUS = 0;
	countMAMA_MSG_STATUS_NOT_ENTITLED = 0;
	countMAMA_MSG_STATUS_NOT_FOUND = 0;
	countMAMA_MSG_STATUS_POSSIBLY_STALE = 0;
	countMAMA_MSG_STATUS_NOT_PERMISSIONED = 0;
	countMAMA_MSG_STATUS_TOPIC_CHANGE = 0;
	countMAMA_MSG_STATUS_BANDWIDTH_EXCEEDED = 0;
	countMAMA_MSG_STATUS_DUPLICATE = 0;
	countMAMA_MSG_STATUS_UNKNOWN = 0;
}

MdsOmEnv::~MdsOmEnv()
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::~MdsOmEnv: %s entry", name);

	wthread_mutex_destroy(&statsLock);

	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::~MdsOmEnv: exit");
}

void MdsOmEnv::init(const MdsOmConfig& cfg, bool enableStats)
{
	this->config = cfg;
	this->type = config.type;
	this->enableStats = enableStats;

	if (config.dictionarySource.size() == 0) {
		config.dictionarySource = "WOMBAT";		// This is the standard for all envs
	}

	switch (type) {
	case MDS_OM_ENV_MERCURY:
		name = "idpb";
		bridgeName = "solace";
		enableDq = false;
		requireInitial = true;
		payloadId = MAMA_PAYLOAD_SOLACE;
		break;
	case MDS_OM_ENV_TREP:
		name = "trep";
		bridgeName = "tick42rmds";
		enableDq = false;
		requireInitial = false;
		payloadId = MAMA_PAYLOAD_TICK42RMDS;
		break;
	case MDS_OM_ENV_MAMACACHE:
		name = "mama";
		bridgeName = "wmw";
		enableDq = true;
		requireInitial = true;
		payloadId = MAMA_PAYLOAD_WOMBAT_MSG;
		break;
	case MDS_OM_ENV_REFLECT:
		name = "refl";
		bridgeName = "reflect";
		enableDq = false;
		requireInitial = true;
		payloadId = MAMA_PAYLOAD_SOLACE;
		// payloadId = MAMA_PAYLOAD_TICK42RMDS;
		break;
	default:
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::init: invalid env=%d", type);
		throw MdsOmStatus(MDS_OM_STATUS_INVALID_ENV);
		break;
	}

	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::init: name=%s bridge=%s dq=%d payload=%c",
		name, bridgeName, enableDq, payloadId);
	config.toString();
	if (config.getCanThisEnvFailToStart()) {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::init: name=%s WARNING this env can fail to start and the app will still continue", name);
	}
}

/**
 * Load this env's source list from the mama.properties
 */
void MdsOmEnv::loadSourceList()
{
	char buf[255];
	string tname = config.transportName;
	if (config.transportNames.size() > 0) {
		config.transportNames.rewind();
		tname = config.transportNames.next();
	}
	snprintf(buf, sizeof(buf), "mama.%s.transport.%s.sourceMap", bridgeName, tname.c_str());
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::loadSourceList: %s %s", name, buf);
	const char* sourceMap = Mama::getProperty(buf);
	if (sourceMap != NULL) {
		// Comma delim list of sources
		char* p = strdup(sourceMap);
		char* last;
		char* seps = (char*) ", \t\r\n";
		for (char* s = STRTOK_MDS(p, seps, &last); s != NULL; s = STRTOK_MDS(NULL, seps, &last)) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "     %s", s);
			if (!strcmp(s, "DEFAULT")) {
				// This env handles any source not found elsewhere
				handlesDefaultSource = true;
			} else {
				sourceEnvList.push_back(s);
			}
		}
		free(p);
	}
}

MamaMsg* MdsOmEnv::getMamaMsg() const
{
	return newMamaMsg(getPayloadId());
}

void MdsOmEnv::destroyMamaMsg(MamaMsg* msg)
{
	deleteMamaMsg(msg);
}

/**
 * See if a source is in the env's source list
 */
bool MdsOmEnv::findSourceList(const char* source)
{
	MdsOmEnvCollection::const_iterator it = sourceEnvList.begin();
	while (it != sourceEnvList.end()) {
		const string& s = *it++;
		// Look for exact match in source
		if (!strcmp(source, s.c_str())) {
			return true;
		}
	}

	// Look for transport-specific mappings
	map<string, MdsOmTransportSet*>::iterator itx = transports.getMap().begin();
	while (itx != transports.getMap().end()) {
		const string& s = itx->first;
		if (!strcmp(source, s.c_str())) return true;
		itx++;
	}

	return false;
}

MamaFieldDescriptor* MdsOmEnv::getFieldDescriptor(mama_fid_t fid)
{
	if (getDictionary() == NULL) {
		// No dictionary
		if (config.loadDictionary == true) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getFieldDescriptor: %s no dictionary", getName());
		}
		return NULL;
	}
	MamaFieldDescriptor* fieldDescriptor = getDictionary()->getFieldByFid(fid);
	if (fieldDescriptor == NULL) {
		// Cannot find this field
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::getFieldDescriptor: %s cannot find field %d", getName(),  fid);
		return NULL;
	}
	return fieldDescriptor;
}

MamaFieldDescriptor* MdsOmEnv::getFieldDescriptor(const char* fieldName)
{
	if (fieldName == NULL) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getFieldDescriptor: %s cannot find field with NULL field name", getName());
		return NULL;
	}
	if (getDictionary() == NULL) {
		if (config.loadDictionary == true) {
			// No dictionary
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getFieldDescriptor: %s no dictionary", getName());
		}
		return NULL;
	}
	MamaFieldDescriptor* fieldDescriptor = getDictionary()->getFieldByName(fieldName);
	if (fieldDescriptor == NULL) {
		// Cannot find this field
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::getFieldDescriptor: %s cannot find field '%s'", getName(), fieldName);
		return NULL;
	}
	return fieldDescriptor;
}

const char* MdsOmEnv::getFieldName(mama_fid_t fid)
{
	if (getDictionary() == NULL) {
		if (config.loadDictionary == true) {
			// No dictionary
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getFieldName: %s no dictionary", getName());
		}
		return NULL;
	}
	MamaFieldDescriptor* fieldDescriptor = getDictionary()->getFieldByFid(fid);
	if (fieldDescriptor == NULL) {
		// Cannot find this field
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::getFieldName: %s cannot find field %d", getName(),  fid);
		return NULL;
	}
	return fieldDescriptor->getName();
}

mama_fid_t MdsOmEnv::getFieldFid(const char* fieldName)
{
	if (fieldName == NULL) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getFieldFid: %s cannot find field with NULL field name", getName());
		return 0;
	}
	if (getDictionary() == NULL) {
		if (config.loadDictionary == true) {
			// No dictionary
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getFieldFid: %s no dictionary", getName());
		}
		return 0;
	}
	MamaFieldDescriptor* fieldDescriptor = getDictionary()->getFieldByName(fieldName);
	if (fieldDescriptor == NULL) {
		// Cannot find this field
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::getFieldFid: %s cannot find field '%s'", getName(), fieldName);
		return 0;
	}
	return fieldDescriptor->getFid();
}

bool MdsOmEnv::writeDictionary(const char* fileName)
{
	if (fileName == NULL) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::writeDictionary: %s NULL file name", getName());
		return false;
	}
	if (getDictionary() == NULL) {
		if (config.loadDictionary == true) {
			// No dictionary
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::writeDictionary: %s no dictionary", getName());
		}
		return false;
	}
	MamaDictionary* d = getDictionary();
	d->writeToFile(fileName);
	return true;
}

void MdsOmEnv::load()
{
	try {
		bridge = Mama::loadBridge(bridgeName);
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::load: %s %s bridge=%p", name, bridgeName, bridge);
		if (bridge == NULL) {
			MamaStatus st(MAMA_STATUS_NO_BRIDGE_IMPL);
			throw st;
		}
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::load: error %s %s %s", name, bridgeName, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOmEnv::open()
{
	try {
		if (config.transportName.empty() && config.transportNames.size() == 0 && config.sourcesMap.size() == 0) {
			// They have to give us a transport name
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::open: %s no transport name provided", name);
			throw MdsOmStatus(MDS_OM_STATUS_TRANSPORT_NOT_PROVIDED);
		}

		loadSourceList();

		// Check for multiple transports, send specific source to each one
		// If the lists are empty then it will default to a single transport
		config.transportListIndex = 0;
		if (config.transportNames.size() > 0) {
			config.roundRobinTransportCount = (int) config.transportNames.size();
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv.open: %s create named transports=%d", name, config.roundRobinTransportCount);

			queueGroup = getQueueGroup(config.roundRobinTransportCount, bridge);

			config.transportNames.rewind();
			while (const char* buf = config.transportNames.next()) {
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s create named transport %s", name, buf);
				countTransport++;
				MamaTransport* transport = getTransport(buf);
				transports.addTransport(transport);
				createTransport(transport, buf);
			}
		} else if (config.sourcesMap.size() == 0 && config.roundRobinTransportCount > 0) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s create round robin transports=%d", name, config.roundRobinTransportCount);

			queueGroup = getQueueGroup(config.roundRobinTransportCount, bridge);

			for (int i = 0; i < config.roundRobinTransportCount; ++i) {
				char buf[32];
				sprintf(buf, "%s-%d", name, i+1);
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s create round robin transport %s", name, buf);
				countTransport++;
				MamaTransport* transport = getTransport(buf);
				transports.addTransport(transport);
				createTransport(transport, config.transportName.c_str());
			}
		} else {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s create source transports=%d", name, config.sourcesMap.size());

			MdsOmSourcesMap::iterator it = config.sourcesMap.begin();
			while (it != config.sourcesMap.end()) {
				string tName = it->first;
				MdsOmSourceCollection sources = it->second;
				it++;

				if (sources.size() == 0) {
					mama_log(MAMA_LOG_LEVEL_WARN, "MdsOmEnv::open: %s found list of %s sources with 0 names", tName.c_str(), name);
					continue;
				}

				// Create N transports for each source group, based on the round robin count
				int transportCount = config.roundRobinTransportCount == 0 ? 1 : config.roundRobinTransportCount;
				countTransport += transportCount;

				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s create source transport %s (%d)", name, tName.c_str(), transportCount);
				vector<MamaTransport*> tmpTransports;
				for (int i = 0; i < transportCount; ++i) {
					MamaTransport* transport = getTransport("ALL");
					tmpTransports.push_back(transport);
				}

				// For each source on the transport add it to the map
				vector<MamaTransport*>::iterator ity;
				MdsOmSourceCollection::iterator itx = sources.begin();
				while (itx != sources.end()) {
					string source = *itx++;
					ity = tmpTransports.begin();
					while (ity != tmpTransports.end()) {
						MamaTransport* transport =  *ity++;
						transports.addTransport(transport, source);
						mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s     add source %s", name, source.c_str());
					}
				}

				// Create after adding to transports collection
				ity = tmpTransports.begin();
				while (ity != tmpTransports.end()) {
					MamaTransport* transport =  *ity++;
					createTransport(transport, tName.c_str());		
				}
			}
			if (transports.isMap()) {
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s create queue group size=%d", name, transports.mapSize());
				queueGroup = getQueueGroup((int) transports.mapSize(), bridge);
			} else {
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s create single transport %s", name, "ALL");
				queueGroup = getQueueGroup(1, bridge);
				countTransport++;
				MamaTransport* transport = getTransport("ALL");
				transports.addTransport(transport);
				createTransport(transport, config.transportName.c_str());
			}
		}
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::open: %s transports=%d", name, countTransport);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::open: error %s %s", name, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MamaQueueGroup* MdsOmEnv::getQueueGroup(int size, mamaBridge bridge)
{
	MamaQueueGroup* queueGroup = new MamaQueueGroup(size, bridge);

	for (int i = 0; i < queueGroup->getNumberOfQueues(); i++) {
		void* end = (void*) calloc(1, sizeof(mama_u64_t));	// use to calc the avg between onMsg times
		MamaQueue* q = queueGroup->getNextQueue();
		q->setClosure(end);
	}

	printQueueGroup(queueGroup);

	return queueGroup;
}

void MdsOmEnv::printQueueGroup(MamaQueueGroup* qg)
{
	for (int i = 0; i < qg->getNumberOfQueues(); i++) {
		MamaQueue* q = qg->getNextQueue();
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::printQueueGroup: queue #%d elements=%d", i+1, q->getEventCount());
	}
}

MamaTransport* MdsOmEnv::getTransport(const char* descr)
{
	MamaTransport* transport = new MamaTransport;
	transport->setTransportCallback(this);				// for MdsOm and user
	transport->setTransportTopicCallback(this);			// for user and publisher events
	transport->setDescription(descr);
	return transport;
}

void MdsOmEnv::createTransport(MamaTransport* transport, const char* name)
{
	transport->create(name, bridge);			// this may fail if user is not valid
}

void MdsOmEnv::stopQueues()
{
	try {
		if (dictionary) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::stopQueues: %s delete dictionary", name);
			if (dictionary) {
				delete dictionary;
				dictionary = NULL;
			}
		}

		if (queueGroup) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::stopQueues: %s group=%p", name, (void*) queueGroup);
			printQueueGroup(queueGroup);

			try {
				for (int i = 0; i < queueGroup->getNumberOfQueues(); i++) {
					MamaQueue* q = queueGroup->getNextQueue();
					void* closure = q->getClosure();
					if (closure) free(closure);
				}
				queueGroup->destroyWait();
				delete queueGroup;
			} catch (MamaStatus& status) {
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::stopQueues: TEMPORARY error %s %s", name, status.toString());
				// Send original exception reference back to caller
			}
			queueGroup = NULL;
		}
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::stopQueues: error %s %s", name, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOmEnv::close()
{
	transports.close();
}

void MdsOmEnv::setDictionaryFile(const char* fileName)
{
	if (fileName == NULL) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::setDictionaryFile: null dictionary file name passed in %s", name);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	config.dictionaryFile = fileName;
}

MdsOmSubscription* MdsOmEnv::snap(const char* topic, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (topic == NULL || cb == NULL || strlen(topic) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::snap: %s null arg passed in cb=%p topic=%p", name, cb, topic);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	pair<string, string> s = getSourceAndSymbol(topic);
	return snap(s.first.c_str(), s.second.c_str(), cb, closure, setup);
}

MdsOmSubscription* MdsOmEnv::snap(const char* source, const char* symbol, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (source == NULL || symbol == NULL || cb == NULL || strlen(source) == 0 || strlen(symbol) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::snap: null arg passed in %s cb=%p source=%s symbol=%s", name, cb, source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	try {
		countSubs++;

		MdsOmSubscription* sub = new MdsOmSubscription;
		sub->snap(this, transports.getTransport(source), source, symbol, cb, closure, setup);

		return sub;

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::snap: error %s %s", name, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MdsOmSubscription* MdsOmEnv::subscribe(const char* topic, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (topic == NULL || cb == NULL || strlen(topic) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::subscribe: %s null arg passed in cb=%p topic=%p", name, cb, topic);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	pair<string, string> s = getSourceAndSymbol(topic);
	return subscribe(s.first.c_str(), s.second.c_str(), cb, closure, setup);
}

MdsOmSubscription* MdsOmEnv::subscribe(const char* source, const char* symbol, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (source == NULL || symbol == NULL || cb == NULL || strlen(source) == 0 || strlen(symbol) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::subscribe: null or empty arg passed in %s cb=%p source=%s symbol=%s", name, cb, source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	// Now trim and check for empty string, some chains have ' ' as the next link
	string src = MdsTrim(source);
	string sym = MdsTrim(symbol);
	if (src == "" || sym == "") {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::subscribe: null or empty arg passed in %s cb=%p source=%s symbol=%s", name, cb, source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	try {
		countSubs++;

		MamaTransport* t = transports.getTransport(source);
		MdsOmSubscription* sub = new MdsOmSubscription;
		sub->create(this, t, src.c_str(), sym.c_str(), cb, closure, setup);

		return sub;

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::subscribe: error %s %s", name, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MdsOmPublisher* MdsOmEnv::getPublisher(const char* topic, MdsOmPublisherCallback* cb, void* closure)
{
	if (topic == NULL || strlen(topic) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getPublisher: %s null arg passed in topic=%p", name, topic);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	pair<string, string> s = getSourceAndSymbol(topic);
	return getPublisher(s.first.c_str(), s.second.c_str(), cb, closure);
}

MdsOmPublisher* MdsOmEnv::getPublisher(const char* source, const char* symbol, MdsOmPublisherCallback* cb, void* closure)
{
	if (source == NULL || symbol == NULL || strlen(source) == 0 || strlen(symbol) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getPublisher: null arg passed in %s source=%s symbol=%s", name, source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	// Now trim and check for empty string
	string src = MdsTrim(source);
	string sym = MdsTrim(symbol);
	if (src == "" || sym == "") {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::getPublisher: null or empty arg passed in %s source=%s symbol=%s", name, source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	if (mama_getLogLevel() >= MAMA_LOG_LEVEL_FINE) mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::getPublisher: %s source=%s symbol=%s", name, source, symbol);

	MdsOmPublisher* pub = NULL;
	try {
		countPubs++;

		pub = new MdsOmPublisher();
		pub->create(this, transports.getTransport(source), src.c_str(), sym.c_str(), cb, closure);
		return pub;
	} catch (MamaStatus& status) {
		if (pub) delete pub;
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::getPublisher: error %s %s.%s %s", name, source, symbol, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOmEnv::getTransports(vector<MamaTransport*>& transportList)
{
	transports.getTransports(transportList);
}

void MdsOmEnv::loadDictionary()
{
	try {
		if (!config.dictionaryFile.empty()) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::loadDictionary: load dictionary for %s from %s", name, config.dictionaryFile.c_str());
			dictionary = new MamaDictionary;
			dictionary->populateFromFile(config.dictionaryFile.c_str());
			dictionaryStatus = MDS_OM_ENV_STATUS_SUCCESS;
		} else if (!config.dictionarySource.empty()) {
			if (config.loadDictionary) {
				MamaTransport* t = transports.getTransport("ALL");
				MamaSource* sourceDictionary = new MamaSource(config.dictionarySource.c_str(), t, config.dictionarySource.c_str());
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::loadDictionary: load dictionary for %s from %s on %s", name, config.dictionarySource.c_str(), t->getName());
				dictionary = new MamaDictionary;
				dictionary->create(queueGroup->getNextQueue(),
									this,
									sourceDictionary,
									3,
									30.0);
				Mama::start(bridge);
				delete sourceDictionary;
			} else {
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::loadDictionary: NO dictionary for %s", name);
				dictionary = NULL;
				dictionaryStatus = MDS_OM_ENV_STATUS_SUCCESS;
			}
		} else {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::loadDictionary: error, load dictionary for %s from neither file nor infra", name);
			throw MdsOmStatus(MDS_OM_STATUS_DICTIONARY_NO_SOURCE);
		}
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmEnv::loadDictionary: error %s %s", name, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

// --- DICTIONARY CB -----------------------------------------------------------------
void MdsOmEnv::onTimeout(void)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onTimeout: timed out waiting for dictionary %s", name);
	dictionaryStatus = MDS_OM_ENV_STATUS_FAIL;
    Mama::stop(bridge);
}

void MdsOmEnv::onError(const char* errMsg)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onError: error for dictionary %s error=%s", name, errMsg);
	dictionaryStatus = MDS_OM_ENV_STATUS_FAIL;
    Mama::stop(bridge);
}

void MdsOmEnv::onComplete(void)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onComplete: Got dictionary %s size=%d maxFid=%d", name, dictionary->getSize(), dictionary->getMaxFid());
	dictionaryStatus = MDS_OM_ENV_STATUS_SUCCESS;
    Mama::stop(bridge);
}

// --- TRANSPORT TOPIC CB -----------------------------------------------------------------
//
void MdsOmEnv::onTopicSubscribe (MamaTransport* tport,
                       const char* topic,
                       const void* platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::onTopicSubscribe: %s %s %s %s", tport->getDescription(), tport->getMiddleware(), tport->getName(), topic);
	if (config.transportTopicCb) config.transportTopicCb->onTopicSubscribe(this, tport, topic, platformInfo);
}

void MdsOmEnv::onTopicUnsubscribe (MamaTransport* tport,
                         const char* topic,
                         const void* platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::onTopicUnsubscribe: %s %s %s %s", tport->getDescription(), tport->getMiddleware(), tport->getName(), topic);
	if (config.transportTopicCb) config.transportTopicCb->onTopicUnsubscribe(this, tport, topic, platformInfo);
}

void MdsOmEnv::onTopicPublishError (MamaTransport* tport,
                            const char* topic,
                            const void* platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::onTopicPublishError: %s %s %s %s", tport->getDescription(), tport->getMiddleware(), tport->getName(), topic);
	if (config.transportTopicCb) config.transportTopicCb->onTopicPublishError(this, tport, topic, platformInfo);
	MdsOmPublisher::feedback.processTransportTopicPublishers(topic, MAMA_STATUS_PLATFORM);
}

void MdsOmEnv::onTopicPublishErrorNotEntitled (MamaTransport* tport,
                                     const char* topic,
                                     const void* platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::onTopicPublishErrorNotEntitled: %s %s %s %s", tport->getDescription(), tport->getMiddleware(), tport->getName(), topic);
	if (config.transportTopicCb) config.transportTopicCb->onTopicPublishErrorNotEntitled(this, tport, topic, platformInfo);
	MdsOmPublisher::feedback.processTransportTopicPublishers(topic, MAMA_STATUS_NOT_ENTITLED);
}

void MdsOmEnv::onTopicPublishErrorBadSymbol (MamaTransport* tport,
                                   const char* topic,
                                   const void* platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmEnv::onTopicPublishErrorBadSymbol: %s %s %s %s", tport->getDescription(), tport->getMiddleware(), tport->getName(), topic);
	if (config.transportTopicCb) config.transportTopicCb->onTopicPublishErrorBadSymbol(this, tport, topic, platformInfo);
	MdsOmPublisher::feedback.processTransportTopicPublishers(topic, MAMA_STATUS_BAD_SYMBOL);
}

// --- TRANSPORT CB -----------------------------------------------------------------
//

#if 0
#include "mama/conflation/connection_int.h"
#else
#define MAX_STR_LEN 67
#define MAX_USER_STR_LEN 256
#define MAMACONNECTION_MAX_IP_ADDRESS_LEN INET_ADDRSTRLEN    
typedef struct mamaConnection_
{
    mamaTransport mTransport;
    char          mIpAddress[MAMACONNECTION_MAX_IP_ADDRESS_LEN];
    uint16_t      mPort;
    uint32_t      mMaxQueueSize;
    uint32_t      mCurQueueSize;
    uint32_t      mMsgCount;
    uint32_t      mBytesSent;
    void*         mHandle;
    char          mStrVal[MAX_STR_LEN];
    char          mUserName[MAX_USER_STR_LEN];
    char          mAppName[MAX_USER_STR_LEN];
} mamaConnectionImpl;
#endif

void MdsOmEnv::logPlatformInfo(MamaTransport* t, const char* msg, const void* platformInfo) const
{
	// This struct only comes from Wombat now, waiting for Tick42 and Solace to implement it
	if (t != NULL && platformInfo != NULL) {
		const char* tname = t->getMiddleware();
		if (tname != NULL && !strcmp(tname, "wmw")) {
			mamaConnectionImpl* p = (mamaConnectionImpl*) platformInfo;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::%s: ip=%*s:%d user=%s source=%s",
				msg, MAMACONNECTION_MAX_IP_ADDRESS_LEN, p->mIpAddress, p->mPort, p->mUserName, p->mAppName);

#if 0
			MamaConnection** list;
			uint32_t len;
			t->getAllConnections(list, len);
			for (int i = 0; i < len; i++) {
				MamaConnection* c = list[i];
				if (c != NULL) {
					mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::%s:        ip=%s:%d user=%s app=%s",
						msg, c->getIpAddress(), c->getPort(), c->getUserName(), c->getAppName());
				}
			}
			t->freeAllConnections(list, len);
#endif
		}
	}
}

void MdsOmEnv::onDisconnect(
    MamaTransport*  transport, 
    const void*     platformInfo)
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onDisconnect: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	transports.setStatus(transport, MDS_OM_ENV_STATUS_FAIL);
	logPlatformInfo(transport, "onDisconnect", platformInfo);
	if (config.transportCb) config.transportCb->onDisconnect(this, transport, platformInfo);
}

void MdsOmEnv::onReconnect(
    MamaTransport*  transport,
    const void*     platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onReconnect: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	transports.setStatus(transport, MDS_OM_ENV_STATUS_SUCCESS);
	logPlatformInfo(transport, "onReconnect", platformInfo);
	if (config.transportCb) config.transportCb->onReconnect(this, transport, platformInfo);
}

void MdsOmEnv::onQuality(
    MamaTransport*     transport,
    short              cause,
    const void*        platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onQuality: %s %s %s cause=%d", transport->getDescription(), transport->getMiddleware(), transport->getName(), cause);
	logPlatformInfo(transport, "onQuality", platformInfo);
	if (config.transportCb) config.transportCb->onQuality(this, transport, cause, platformInfo);
}

void MdsOmEnv::onConnect(
    MamaTransport*  transport,
    const void*     platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onConnect: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	transports.setStatus(transport, MDS_OM_ENV_STATUS_SUCCESS);
	logPlatformInfo(transport, "onConnect", platformInfo);
	if (config.transportCb) config.transportCb->onConnect(this, transport, platformInfo);
}

void MdsOmEnv::onAccept(
    MamaTransport*   transport,
    const void*      platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onAccept: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	transports.setStatus(transport, MDS_OM_ENV_STATUS_SUCCESS);
	logPlatformInfo(transport, "onAccept", platformInfo);
	if (config.transportCb) config.transportCb->onAccept(this, transport, platformInfo);
}

void MdsOmEnv::onAcceptReconnect(
    MamaTransport*  transport,
    const void*     platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onAcceptReconnect: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	transports.setStatus(transport, MDS_OM_ENV_STATUS_SUCCESS);
	logPlatformInfo(transport, "onAcceptReconnect", platformInfo);
	if (config.transportCb) config.transportCb->onAcceptReconnect(this, transport, platformInfo);
}

void MdsOmEnv::onPublisherDisconnect(
    MamaTransport*  transport,
    const void*     platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onPublisherDisconnect: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	logPlatformInfo(transport, "onPublisherDisconnect", platformInfo);
	if (config.transportCb) config.transportCb->onPublisherDisconnect(this, transport, platformInfo);
}

void MdsOmEnv::onNamingServiceConnect(
    MamaTransport*  transport,
    const void*     platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onNamingServiceConnect: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	transports.setStatus(transport, MDS_OM_ENV_STATUS_SUCCESS);
	logPlatformInfo(transport, "onNamingServiceConnect", platformInfo);
	if (config.transportCb) config.transportCb->onNamingServiceConnect(this, transport, platformInfo);
}

void MdsOmEnv::onNamingServiceDisconnect(
    MamaTransport*  transport,
    const void*     platformInfo)
{
    mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmEnv::onNamingServiceDisconnect: %s %s %s", transport->getDescription(), transport->getMiddleware(), transport->getName());
	transports.setStatus(transport, MDS_OM_ENV_STATUS_FAIL);
	logPlatformInfo(transport, "onNamingServiceDisconnect", platformInfo);
	if (config.transportCb) config.transportCb->onNamingServiceDisconnect(this, transport, platformInfo);
}

// -------------------------------------------------------------------------------
void MdsOmEnv::countOnQuality(mamaQuality quality) {
	switch (quality) {
	case MAMA_QUALITY_MAYBE_STALE:
    case MAMA_QUALITY_STALE:
    case MAMA_QUALITY_PARTIAL_STALE:
    case MAMA_QUALITY_FORCED_STALE:
		countMAMA_MSG_STATUS_STALE++;
		break;
	}
}
		
void MdsOmEnv::countTypes(mamaMsgType type, mamaMsgStatus status) {
	switch (type) {
	case MAMA_MSG_TYPE_UPDATE:
		countMAMA_MSG_TYPE_UPDATE++;
		break;
	case MAMA_MSG_TYPE_INITIAL:
		countMAMA_MSG_TYPE_INITIAL++;
		break;
	case MAMA_MSG_TYPE_CANCEL:
		countMAMA_MSG_TYPE_CANCEL++;
		break;
	case MAMA_MSG_TYPE_ERROR:
		countMAMA_MSG_TYPE_ERROR++;
		break;
	case MAMA_MSG_TYPE_CORRECTION:
		countMAMA_MSG_TYPE_CORRECTION++;
		break;
	case MAMA_MSG_TYPE_CLOSING:
		countMAMA_MSG_TYPE_CLOSING++;
		break;
	case MAMA_MSG_TYPE_RECAP:
		countMAMA_MSG_TYPE_RECAP++;
		break;
	case MAMA_MSG_TYPE_DELETE:
		countMAMA_MSG_TYPE_DELETE++;
		break;
	case MAMA_MSG_TYPE_EXPIRE:
		countMAMA_MSG_TYPE_EXPIRE++;
		break;
	case MAMA_MSG_TYPE_SNAPSHOT:
		countMAMA_MSG_TYPE_SNAPSHOT++;
		break;
	case MAMA_MSG_TYPE_PREOPENING:
		countMAMA_MSG_TYPE_PREOPENING++;
		break;
	case MAMA_MSG_TYPE_QUOTE:
		countMAMA_MSG_TYPE_QUOTE++;
		break;
	case MAMA_MSG_TYPE_TRADE:
		countMAMA_MSG_TYPE_TRADE++;
		break;
	case MAMA_MSG_TYPE_ORDER:
		countMAMA_MSG_TYPE_ORDER++;
		break;
	case MAMA_MSG_TYPE_BOOK_INITIAL:
		countMAMA_MSG_TYPE_BOOK_INITIAL++;
		break;
	case MAMA_MSG_TYPE_BOOK_UPDATE:
		countMAMA_MSG_TYPE_BOOK_UPDATE++;
		break;
	case MAMA_MSG_TYPE_BOOK_CLEAR:
		countMAMA_MSG_TYPE_BOOK_CLEAR++;
		break;
	case MAMA_MSG_TYPE_BOOK_RECAP:
		countMAMA_MSG_TYPE_BOOK_RECAP++;
		break;
	case MAMA_MSG_TYPE_BOOK_SNAPSHOT:
		countMAMA_MSG_TYPE_BOOK_SNAPSHOT++;
		break;
	case MAMA_MSG_TYPE_NOT_PERMISSIONED:
		countMAMA_MSG_TYPE_NOT_PERMISSIONED++;
		break;
	case MAMA_MSG_TYPE_NOT_FOUND:
		countMAMA_MSG_TYPE_NOT_FOUND++;
		break;
	case MAMA_MSG_TYPE_END_OF_INITIALS:
		countMAMA_MSG_TYPE_END_OF_INITIALS++;
		break;
	case MAMA_MSG_TYPE_WOMBAT_REQUEST:
		countMAMA_MSG_TYPE_WOMBAT_REQUEST++;
		break;
	case MAMA_MSG_TYPE_WOMBAT_CALC:
		countMAMA_MSG_TYPE_WOMBAT_CALC++;
		break;
	case MAMA_MSG_TYPE_SEC_STATUS:
		countMAMA_MSG_TYPE_SEC_STATUS++;
		break;
	case MAMA_MSG_TYPE_DDICT_SNAPSHOT:
		countMAMA_MSG_TYPE_DDICT_SNAPSHOT++;
		break;
	case MAMA_MSG_TYPE_MISC:
		countMAMA_MSG_TYPE_MISC++;
		break;
	case MAMA_MSG_TYPE_TIBRV:
		countMAMA_MSG_TYPE_TIBRV++;
		break;
	case MAMA_MSG_TYPE_FEATURE_SET:
		countMAMA_MSG_TYPE_FEATURE_SET++;
		break;
	case MAMA_MSG_TYPE_SYNC_REQUEST:
		countMAMA_MSG_TYPE_SYNC_REQUEST++;
		break;
	case MAMA_MSG_TYPE_REFRESH:
		countMAMA_MSG_TYPE_REFRESH++;
		break;
	case MAMA_MSG_TYPE_WORLD_VIEW:
		countMAMA_MSG_TYPE_WORLD_VIEW++;
		break;
	case MAMA_MSG_TYPE_NEWS_QUERY:
		countMAMA_MSG_TYPE_NEWS_QUERY++;
		break;
	case MAMA_MSG_TYPE_NULL:
		countMAMA_MSG_TYPE_NULL++;
		break;
	}

	switch (status) {
	case MAMA_MSG_STATUS_OK:
		countMAMA_MSG_STATUS_OK++;
		break;
	case MAMA_MSG_STATUS_LINE_DOWN:
		countMAMA_MSG_STATUS_LINE_DOWN++;
		break;
	case MAMA_MSG_STATUS_NO_SUBSCRIBERS:
		countMAMA_MSG_STATUS_NO_SUBSCRIBERS++;
		break;
	case MAMA_MSG_STATUS_BAD_SYMBOL:
		countMAMA_MSG_STATUS_BAD_SYMBOL++;
		break;
	case MAMA_MSG_STATUS_EXPIRED:
		countMAMA_MSG_STATUS_EXPIRED++;
		break;
	case MAMA_MSG_STATUS_TIMEOUT:
		countMAMA_MSG_STATUS_TIMEOUT++;
		break;
	case MAMA_MSG_STATUS_MISC:
		countMAMA_MSG_STATUS_MISC++;
		break;
	case MAMA_MSG_STATUS_STALE:
		countMAMA_MSG_STATUS_STALE++;
		break;
	case MAMA_MSG_STATUS_PLATFORM_STATUS:
		countMAMA_MSG_STATUS_PLATFORM_STATUS++;
		break;
	case MAMA_MSG_STATUS_NOT_ENTITLED:
		countMAMA_MSG_STATUS_NOT_ENTITLED++;
		break;
	case MAMA_MSG_STATUS_NOT_FOUND:
		countMAMA_MSG_STATUS_NOT_FOUND++;
		break;
	case MAMA_MSG_STATUS_POSSIBLY_STALE:
		countMAMA_MSG_STATUS_POSSIBLY_STALE++;
		break;
	case MAMA_MSG_STATUS_NOT_PERMISSIONED:
		countMAMA_MSG_STATUS_NOT_PERMISSIONED++;
		break;
	case MAMA_MSG_STATUS_TOPIC_CHANGE:
		countMAMA_MSG_STATUS_TOPIC_CHANGE++;
		break;
	case MAMA_MSG_STATUS_BANDWIDTH_EXCEEDED:
		countMAMA_MSG_STATUS_BANDWIDTH_EXCEEDED++;
		break;
	case MAMA_MSG_STATUS_DUPLICATE:
		countMAMA_MSG_STATUS_DUPLICATE++;
		break;
	case MAMA_MSG_STATUS_UNKNOWN:
		countMAMA_MSG_STATUS_UNKNOWN++;
		break;
	}
}

}
