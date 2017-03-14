
#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

// ----------------------------------------------------------------------
MdsOm::MdsOm()
{
	init(NULL, MAMA_LOG_LEVEL_NORMAL);
}

MdsOm::MdsOm(const char* logFile, MamaLogLevel level)
{
	init(logFile, level);
}

// ----------------------------------------------------------------------
int MdsOm::getWaits() const { return waits; }

void MdsOm::setWaits(int waits) {
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::setWaits: %d", waits);
	this->waits = waits;
}

bool MdsOm::getFullTimeout() const { return fullTimeout; }

void MdsOm::setFullTimeout(bool fullTimeout) {
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::setFullTimeout: %d", fullTimeout);
	this->fullTimeout = fullTimeout;
}

const char* MdsOm::getLogFile() const { return logFile; }

int MdsOm::getStatsInterval() const { return statsInterval; }

void MdsOm::setStatsInterval(int statsInterval) {
	this->statsInterval = statsInterval;
}

void MdsOm::setMultipleEnvs()
{
	multipleEnvs = true;
}

// ----------------------------------------------------------------------
void MdsOm::init(const char* logFile, MamaLogLevel level)
{
	this->logFile = NULL;
	if (logFile) {
		this->logFile = strdup(logFile);
	}

	multipleEnvs = false;
	idp = NULL;
	rmds = NULL;
	firefly = NULL;
	reflect = NULL;
	statsInterval = 0;			 // default to 0 secs (never)
	waits = 30;					 // default to 30 wait cycles
	fullTimeout = false;
	didStart = false;

	wsem_init(&doneSem, 0, 0);

	// This turns on logging, and will be redone w/ mama.properties when open is called
	logFileFp = stderr;
	if (logFile) {
		logFileFp = fopen(logFile, "w");
		if (logFileFp == NULL) {
			fprintf(stderr, "MdsOm: cannot open log file %s\n", logFile);
			logFileFp = stderr;
		}
	}
	mama_enableLogging(logFileFp, level);

	const char* envVars[] = {
		"WOMBAT_PATH",
		"OPENMAMA_HOME",
		"LD_LIBRARY_PATH",
		"PATH",
		"HOSTNAME",
		"COMPUTERNAME",
		"JAVA_HOME",
		NULL
	};

	for (int i = 0; envVars[i] != NULL; i++) {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm: %s=%s", envVars[i], getenv(envVars[i]) ? getenv(envVars[i]) : "");
	}

	const char* username = "(none)";
	mama_status status = mama_getUserName(&username);
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm: mama_getUserName=%s %d", username, status);
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm: waits=%d", waits);

#ifndef WIN32
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
}

MdsOm::~MdsOm()
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::~MdsOm: entry");

	MdsOmEnvsCollection::iterator it = envs.begin();
	while (it != envs.end()) {
		MdsOmEnv* env = *it++;
		if (env) delete env;
	}

	if (logFile) free((void*) logFile);		// free our copy
	// Don't close logFileFp since mama does that on close
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::~MdsOm: exit");
}

MdsOmEnv* MdsOm::addEnv(MdsOmConfig& config)
{
	MdsOmEnv* env = NULL;

	if (config.type == MDS_OM_ENV_UNKNOWN) {
		// Check bridgeName
		if (config.bridgeName == "solace") {
			config.type = MDS_OM_ENV_MERCURY;
		} else if (config.bridgeName == "tick42rmds") {
			config.type = MDS_OM_ENV_TREP;
		} else if (config.bridgeName == "wmw") {
			config.type = MDS_OM_ENV_MAMACACHE;
		}
	}

	switch (config.type) {
	case MDS_OM_ENV_MERCURY:
		if (idp && !multipleEnvs) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::addEnv: %s already have Mercury env added", config.transportName.c_str());
			throw MdsOmStatus(MDS_OM_STATUS_INVALID_CONFIG);
		}

		// Solace supports 1 transport per transport name.
		// If an app wants multiple transport to an appliance they can create multiple entries in mama.properties, but MdsOm does not currently support that.
		// For performance a single transport to an appliance for PB data will be fine.
		if (config.roundRobinTransportCount > 1 || config.sourcesMap.size() > 0) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::addEnv: %s Mercury/Solace only supports 1 transport per transport name - roundRobin=%d and list=%d",
				config.transportName.c_str(), config.roundRobinTransportCount, config.sourcesMap.size());
			throw MdsOmStatus(MDS_OM_STATUS_INVALID_CONFIG);
		}

		env = idp = new MdsOmEnv();
		break;
	case MDS_OM_ENV_TREP:
		if (rmds && !multipleEnvs) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::addEnv: %s already have RMDS/TREP env added", config.transportName.c_str());
			throw MdsOmStatus(MDS_OM_STATUS_INVALID_CONFIG);
		}

		env = rmds = new MdsOmEnv();
		break;
	case MDS_OM_ENV_MAMACACHE:
		if (firefly && !multipleEnvs) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::addEnv: %s already have Firefly env added", config.transportName.c_str());
			throw MdsOmStatus(MDS_OM_STATUS_INVALID_CONFIG);
		}

		env = firefly = new MdsOmEnv();
		break;
	case MDS_OM_ENV_REFLECT:
		if (reflect && !multipleEnvs) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::addEnv: %s already have Reflect env added", config.transportName.c_str());
			throw MdsOmStatus(MDS_OM_STATUS_INVALID_CONFIG);
		}

		env = reflect = new MdsOmEnv();
		break;
	default:
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::addEnv: invalid env requested type=%d", config.type);
		throw MdsOmStatus(MDS_OM_STATUS_INVALID_ENV);
		break;
	}

	bool enableStats = statsInterval > 0 ? true : false;

	env->init(config, enableStats);
	envs.push_back(env);

	return env;
}

MdsOmEnv* MdsOm::getEnv(MdsOmEnvType type) const
{
	switch (type) {
	case MDS_OM_ENV_MERCURY:
		return idp;
		break;
	case MDS_OM_ENV_TREP:
		return rmds;
		break;
	case MDS_OM_ENV_MAMACACHE:
		return firefly;
		break;
	case MDS_OM_ENV_REFLECT:
		return reflect;
		break;
	default:
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::getEnv: invalid env requested type=%d", type);
		throw MdsOmStatus(MDS_OM_STATUS_INVALID_ENV);
		break;
	}
}

// Just need a queue from one of the bridges for a timer
MamaQueue* MdsOm::getTimerQueue() const
{
	MdsOmEnvsCollection::const_iterator it;
	for (it = envs.begin(); it != envs.end(); it++) {
		MdsOmEnv* e = *it;
		return e->getQueue();
	}
	mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::getTimerQueue: no queues available envs=%d.", envs.size());
	throw MdsOmStatus(MDS_OM_STATUS_NO_QUEUES_AVAILABLE);
}

MdsOmEnv* MdsOm::getEnvFromSymbol(const char* topic) const
{
	if (topic == NULL) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::getEnvFromSymbol: null arg passed topic=%s", topic);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	// Figure out which env handles this source
	try {
		pair<string, string> s = getSourceAndSymbol(topic);
		const char* source = s.first.c_str();
		MdsOmEnv* env = findEnv(envs, source);
		return env;

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::getEnvFromSymbol: error topic=%s %s", topic, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MdsOmSubscription* MdsOm::snap(const char* topic, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (topic == NULL || cb == NULL || strlen(topic) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::snap: null arg passed in cb=%p topic=%s", cb, topic);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	pair<string, string> s = getSourceAndSymbol(topic);
	return snap(s.first.c_str(), s.second.c_str(), cb, closure, setup);
}

MdsOmSubscription* MdsOm::snap(const char* source, const char* symbol, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (source == NULL || symbol == NULL || cb == NULL || strlen(source) == 0 || strlen(symbol) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::snap: null arg passed in cb=%p source=%s symbol=%s", cb, source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	// Figure out which env handles this source
	try {
		MdsOmEnv* env = findEnv(envs, source);

		if (env == NULL) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::snap: cannot find source=%s symbol=%s", source, symbol);
			throw MdsOmStatus(MDS_OM_STATUS_CANNOT_FIND_SOURCE_IN_CONFIG);
		}

		return env->snap(source, symbol, cb, closure, setup);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::snap: error source=%s symbol=%s %s", source, symbol, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MdsOmSubscription* MdsOm::subscribe(const char* topic, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (topic == NULL || cb == NULL || strlen(topic) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::subscribe: null arg passed in cb=%p topic=%s", cb, topic);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	pair<string, string> s = getSourceAndSymbol(topic);
	return subscribe(s.first.c_str(), s.second.c_str(), cb, closure, setup);
}

MdsOmSubscription* MdsOm::subscribe(const char* source, const char* symbol, MamaSubscriptionCallback* cb, void* closure, bool setup)
{
	if (source == NULL || symbol == NULL || cb == NULL || strlen(source) == 0 || strlen(symbol) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::subscribe: null arg passed in cb=%p source=%s symbol=%s", cb, source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	// Figure out which env handles this source
	try {
		MdsOmEnv* env = findEnv(envs, source);

		if (env == NULL) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::subscribe: cannot find source=%s symbol=%s", source, symbol);
			throw MdsOmStatus(MDS_OM_STATUS_CANNOT_FIND_SOURCE_IN_CONFIG);
		}

		return env->subscribe(source, symbol, cb, closure, setup);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::subscribe: error source=%s symbol=%s %s", source, symbol, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MdsOmEnv* MdsOm::findEnv(MdsOmEnvsCollection envs, const char* source) const
{
	MdsOmEnv* env = NULL;
	MdsOmEnv* defaultEnv = NULL;
	if (envs.size() == 1) {
		// Only 1 env, just default to that one
		env = envs.at(0);
	} else {
		MdsOmEnvsCollection::const_iterator it;
		for (it = envs.begin(); it != envs.end(); it++) {
			env = *it;
			bool b = env->findSourceList(source);
			if (b) break;
			if (env->handleDefaultSource()) defaultEnv = env;
		}
	}
	return env != NULL ? env : defaultEnv;
}

MdsOmPublisher* MdsOm::getPublisher(const char* topic, MdsOmPublisherCallback* cb, void* closure)
{
	if (topic == NULL || strlen(topic) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::getPublisher: null arg passed in topic=%p", topic);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	pair<string, string> s = getSourceAndSymbol(topic);
	return getPublisher(s.first.c_str(), s.second.c_str(), cb, closure);
}

MdsOmPublisher* MdsOm::getPublisher(const char* source, const char* symbol, MdsOmPublisherCallback* cb, void* closure)
{
	if (source == NULL || symbol == NULL || strlen(source) == 0 || strlen(symbol) == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::getPublisher: null arg passed in source=%s symbol=%s", source, symbol);
		throw MdsOmStatus(MDS_OM_STATUS_NULL_ARG);
	}

	try {
		MdsOmEnv* env = findEnv(envs, source);

		if (env == NULL) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::getPublisher: cannot find source=%s symbol=%s", source, symbol);
			throw MdsOmStatus(MDS_OM_STATUS_CANNOT_FIND_SOURCE_IN_CONFIG);
		}

		return env->getPublisher(source, symbol, cb, closure);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_FINE, "MdsOm::getPublisher: error %s.%s %s", source, symbol, status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOm::open(const char* path, const char* filename)
{
	try {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: envs=%d path=%s filename=%s waits=%d fullTO=%d",
			envs.size(), path, filename, waits, fullTimeout);

		if (path || filename) {
			mama_status status = mama_setPropertiesFromFile(path, filename);
			if (status != MAMA_STATUS_OK) {
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: mama_setPropertiesFromFile failed %s/%d",
					mamaStatus_stringForStatus(status), status);
				throw MdsOmStatus(MDS_OM_STATUS_PROPERTIES_FILE_NOT_FOUND);
			}
		}

		if (envs.size() == 0) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: no environments configured");
			throw MdsOmStatus(MDS_OM_STATUS_NO_ENVS);
		}

		MdsOmEnvsCollection::const_iterator it;
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: env %s", e->getName());
		}

		// -----------------------------------------------------------
		// Load bridges
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: load envs");
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: load env %s", e->getName());
			e->load();
		}

		// -----------------------------------------------------------
		// Open MAMA
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: open mama");
		Mama::open();

		// -----------------------------------------------------------
		// Open env bridges
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: open envs");
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: open env %s", e->getName());
			try {
				e->open();
			} catch (MamaStatus& status) {
				if (e->getCanThisEnvFailToStart()) {
					mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: error %s, but allowed error, continue", status.toString());
				} else {
					mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: error %s", status.toString());
					throw status;
				}
			}
		}

		// Now wait for the envs to connect.
		int localWaits = 0;
		while (true) {
			int waiting = 0;
			int countConnected = 0;
			for (it = envs.begin(); it != envs.end(); it++) {
				MdsOmEnv* e = *it;
				if (e->isConnectWait()) waiting++;
				if (e->isConnected()) countConnected++;
			}

			if (countConnected == envs.size()) {
				// All connected OK
				break;
			}

			if (!fullTimeout && waiting == 0) {
				// Return error if some/all connections failed and not configured to wait for full timeout
				break;
			}

			localWaits++;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: waiting for connection count=%d", localWaits);
			if (waits > 0 && localWaits > waits) {
				// We have timed out
				int failed = 0;
				for (it = envs.begin(); it != envs.end(); it++) {
					MdsOmEnv* e = *it;
					// Log which env did not connect
					if (e->isConnectWait() && !e->getCanThisEnvFailToStart()) {
						failed++;
						mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: env did not connect %s lw=%d w=%d",
							e->getName(), localWaits, waits);
					}
				}
				// Report the fail to connect
				if (failed) {
					throw MdsOmStatus(MDS_OM_STATUS_BRIDGE_TIMEOUT);
				} else {
					break;
				}
			}
			sleep(2);
		}

		int countFailed = 0;
		bool allMustSucceed = true;
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			if (e->isConnectFail()) {
				countFailed++;
				if (e->config.getCanThisEnvFailToStart() == true) allMustSucceed = false;
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: env failed to open %s", e->getName());
			}
		}
		if (countFailed > 0) {
			// One or more failed
			if (countFailed == envs.size() || allMustSucceed) {
				// They all failed or all must have succeeded
				throw MdsOmStatus(MDS_OM_STATUS_BRIDGE_DID_NOT_CONNECT);
			}
		}

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: all/some envs connected total=%d failed=%d", envs.size(), countFailed);

		// -----------------------------------------------------------
		// Load dictionaries
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: load dictionaries");
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: load dictionary %s", e->getName());
			if (e->isConnected()) {
				e->loadDictionary();
			}
		}

		localWaits = 0;
		while (true) {
			int waiting = 0;
			for (it = envs.begin(); it != envs.end(); it++) {
				MdsOmEnv* e = *it;
				if (e->isConnected()) {
					if (e->isDictionaryWait()) waiting++;
				}
			}
			if (waiting == 0) {
				break;
			}
			localWaits++;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: waiting for dictionary count=%d", localWaits);
			if (localWaits > waits) {
				for (it = envs.begin(); it != envs.end(); it++) {
					MdsOmEnv* e = *it;
					if (e->isConnected()) {
						if (e->isDictionaryWait() && !e->getCanThisEnvFailToStart()) {
							mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: dictionary did not load %s", e->getName());
						}
					}
				}
				throw MdsOmStatus(MDS_OM_STATUS_DICTIONARY_TIMEOUT);
			}
			sleep(2);
		}

		bool isDictionaryConnectError = false;
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			if (e->isConnected()) {
				if (e->isDictionaryFail() && !e->getCanThisEnvFailToStart()) {
					isDictionaryConnectError = true;
					mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: env failed to load dictionary %s", e->getName());
				}
			}
		}
		if (isDictionaryConnectError) {
			throw MdsOmStatus(MDS_OM_STATUS_DICTIONARY_DID_NOT_CONNECT);
		}

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::open: all envs OK");

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	} catch (MdsOmStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOm::start(bool allBg)
{
	try {
		this->allBg = allBg;

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::start: allBg=%d size=%d", allBg, envs.size());

		if (envs.size() == 0) {
			mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::open: no environments configured");
			throw MdsOmStatus(MDS_OM_STATUS_NO_ENVS);
		}

		// -----------------------------------------------------------
		// Start background bridges
		didStart = true;			// note that we called start, so we can call stop later
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::start: start bg envs count=%d", envs.size());
		int count = 0;
		MdsOmEnv* fgBridge = NULL;
		MdsOmEnvsCollection::iterator it;
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			if (allBg == false && ++count == envs.size()) {
				// Keep last bridge in list for foreground
				fgBridge = e;
			} else {
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::start: start bg env %s", e->getBridgeName());
				Mama::startBackground(e->getBridge(), this);
			}
		}

		if (statsInterval > 0) {
			// TODO get a queue for just the timer so the timer events are not stuck behind data
			MamaQueue* timerQueue = getTimerQueue();
			statsTimer.create(timerQueue, this, (mama_f64_t) statsInterval, NULL);
		}

		// -----------------------------------------------------------
		// Check fg/bg mode
		if (allBg)  {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::start: all background bridges started, returning");
			return;
		}

		// This blocks
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::start: start fg env %s", fgBridge->getBridgeName());
		Mama::start(fgBridge->getBridge());

		// -----------------------------------------------------------
		// Stopped, clean up
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::start: fg env %s exited", fgBridge->getBridgeName());

		cleanup();

		wsem_post(&doneSem);

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::start: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	} catch (MdsOmStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::start: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOm::cleanup()
{
	try {
		// -----------------------------------------------------------
		// Stopped, clean up

		if (statsInterval > 0) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::cleanup: destroy stats");
			statsTimer.destroy();
		}

		sleep(1);

		if (didStart) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::cleanup: close envs count=%d", envs.size());
			MdsOmEnvsCollection::iterator it;
			for (it = envs.begin(); it != envs.end(); it++) {
				MdsOmEnv* e = *it;
				e->close();
			}
			sleep(1);
		}

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::cleanup: Mama::close");
		Mama::close();

		MamaReservedFields::uninitReservedFields();

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::cleanup: all done");

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::cleanup: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	} catch (MdsOmStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::cleanup: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOm::stopQueues()
{
	try {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::stopQueues: close envs count=%d", envs.size());
		MdsOmEnvsCollection::iterator it;
		for (it = envs.begin(); it != envs.end(); it++) {
			MdsOmEnv* e = *it;
			e->stopQueues();
		}
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::stopQueues: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	} catch (MdsOmStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::stopQueues: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

void MdsOm::close()
{
	try {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::close:");

		stopQueues();

		if (didStart) {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::close: stopAll");
			Mama::stopAll();
		} else {
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::close: did not call start, not calling stopAll");
		}

		if (!didStart || allBg) {
			cleanup();
		} else {
			// Ran with 1 bridge in fg, wait for it to exit
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::close: wait for sem");
			wsem_wait(&doneSem);
		}

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::close: exit");

	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOm::close: error %s", status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

// --- START CALLBACK --------------------------------------
void MdsOm::onStartComplete (MamaStatus status)
{
	// Solace bridge does not send this until later, after the bridge is stopped
	// mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOm::onStartComplete: status %s", status.toString());
}

// --- TIMER CALLBACK --------------------------------------
void MdsOm::onTimer(MamaTimer* timer)
{
	char elapsed[64];
	getElapsed(elapsed);
	mama_f64_t divider = 1000000;

	MdsOmMemVals mv;
	mdsOmGetMemVals(getpid(), &mv);
	double residentMem = mv.rss;
	double sharedMem = mv.vsize;

	mama_log(MAMA_LOG_LEVEL_NORMAL, "");
	mama_log(MAMA_LOG_LEVEL_NORMAL, "STATS: process mem=%.0lf/%.0lf / elapsed %s", residentMem, sharedMem, elapsed);

	MdsOmEnvsCollection::iterator it;
	for (it = envs.begin(); it != envs.end(); it++) {
		MdsOmEnv* e = *it;

		// Message rate
		double interval = statsInterval;

		size_t rate = (size_t) (e->countSubMsgsPeriod / interval);
		e->countSubMsgsPeriod = 0;
		if (rate > e->maxSubMsgRate) e->maxSubMsgRate = rate;

		size_t pubRate = (size_t) (e->countPubMsgsPeriod / interval);
		e->countPubMsgsPeriod = 0;
		if (pubRate > e->maxPubMsgRate) e->maxPubMsgRate = pubRate;

		mama_f64_t inOnMsgAvg = e->inOnMsgSum / (e->inOnMsgCount == 0 ? 1 : e->inOnMsgCount);
		mama_f64_t btwnOnMsgAvg = e->btwnOnMsgSum / (e->btwnOnMsgCount == 0 ? 1 : e->btwnOnMsgCount);

		if (inOnMsgAvg > e->maxInOnMsgAvg) e->maxInOnMsgAvg = inOnMsgAvg;
		if (btwnOnMsgAvg > e->maxBtwnOnMsgAvg) e->maxBtwnOnMsgAvg = btwnOnMsgAvg;

		e->inOnMsgSum = 0;
		e->inOnMsgCount = 0;
		e->btwnOnMsgSum = 0;
		e->btwnOnMsgCount = 0;

		mama_log(MAMA_LOG_LEVEL_NORMAL, "STATS: %s s=%d/%d/%d p=%d/%d/%d in=%.4lf/%.4lf btwn=%.4lf/%.4lf q=%d/%d",
			e->getName(),
			e->countSubs, rate, e->maxSubMsgRate,
			e->countPubs, pubRate, e->maxPubMsgRate,
			inOnMsgAvg / divider, e->maxInOnMsgAvg / divider,
			btwnOnMsgAvg / divider, e->maxBtwnOnMsgAvg / divider,
			e->getQueue()->getEventCount(), e->maxQueueCount);

		string spaces(20, ' ');
		spaces[strlen(e->getName())] = '\0';
		mama_log(MAMA_LOG_LEVEL_NORMAL, "STATS: %s p=%d i=%d u=%d q=%d r=%d rec=%d np=%d nf=%d / bad=%d to=%d stale=%d ne=%d np=%d nf=%d",
			spaces.c_str(),
			e->countPubMsgs,
			e->countMAMA_MSG_TYPE_INITIAL, e->countMAMA_MSG_TYPE_UPDATE, e->countMAMA_MSG_TYPE_QUOTE, e->countResponses,
			e->countMAMA_MSG_TYPE_RECAP, e->countMAMA_MSG_TYPE_NOT_PERMISSIONED, e->countMAMA_MSG_TYPE_NOT_FOUND,
			e->countMAMA_MSG_STATUS_BAD_SYMBOL, e->countMAMA_MSG_STATUS_TIMEOUT, e->countMAMA_MSG_STATUS_STALE,
			e->countMAMA_MSG_STATUS_NOT_ENTITLED, e->countMAMA_MSG_STATUS_NOT_PERMISSIONED, e->countMAMA_MSG_STATUS_NOT_FOUND);
	}
}

void MdsOm::onDestroy(MamaTimer* timer, void* closure)
{
}

}
