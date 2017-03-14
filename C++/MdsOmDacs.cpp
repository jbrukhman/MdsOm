
#include "MdsOmDacs.h"

namespace MdsOm
{

MdsOmDacs::MdsOmDacs()
{
	waitTime = 2;
	useMap = false;		// choose DACS host by source map or round-robin
	hostIndex = 0;

	// TODO
	usageLog = AuthorizationRequest::AlwaysPerformUsageLogging;
    reqType = AuthorizationAgent::TYPE_NORMAL;
}

std::string MdsOmDacs::getLoginKey(std::string host, std::string user)
{
	return host + "." + user;
}
	
bool MdsOmDacs::addHost(std::string host)
{
	if (host.length() == 0) return false;
	hostList.push_back(host);
	return true;
}

/**
 * Client's processEvent() implementation, this method is called back
 * when calling the dispatch() method on the event queue 
 * and there is at least one Event instance in the event queue. 
 */	
void MdsOmDacs::processEvent(const Event& event) 
{
	// Get type of processed event.
	int eventType = event.getType();

	// Process only DACS related event type.
	if (eventType == RFA_DACS_PACKAGE) {
		// Get handle from event instance.
		// Handle *pHandle = event.getHandle();
		const AuthorizationAgentEvent& authEvent = static_cast<const AuthorizationAgentEvent &>(event);
		const AuthorizationAgentEventStatus& status = authEvent.getStatus();

		// Since this client process multiconnection, then get the indicator from closure.
		MdsOmDacsUser* du = (MdsOmDacsUser*) event.getClosure();

		// Get user's state from status instance.
		AuthorizationAgentEventStatus::State state = status.getState();
		int newEntitlementState = SUBSCRIBER_UNKNOWN;
		if (state == AuthorizationAgentEventStatus::LoggedIn) {
			newEntitlementState = SUBSCRIBER_LOGGEDIN;
		} else if (state == AuthorizationAgentEventStatus::LoggedOut) {
			newEntitlementState = SUBSCRIBER_LOGGEDOUT;
		}

		std::string str = du->host + "/" + du->user;
		onEntitlementStateChanged(str, newEntitlementState);

		AuthorizationAgentEventStatus::StatusCode code = status.getStatusCode();
		if (code == AuthorizationAgentEventStatus::DoRepermission) {
			onDoRepermissionEvent(str, authEvent);
		} else if (code == AuthorizationAgentEventStatus::AuthorizationSystemNotAvailable) {
			onAuthorizationSystemNotAvailableEvent(str, authEvent);
		} else if (code == AuthorizationAgentEventStatus::LoginDenied) {
			onLoginDeniedEvent(str, authEvent);
		} else if (code == AuthorizationAgentEventStatus::LogoutForced) {
			onLogOutForcedEvent(str, authEvent);
		}

		du->authDone = true;
	}
}

void MdsOmDacs::log(std::string s)
{
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%Y-%m-%s %I:%M:%S", timeinfo);
	
	printf("%s: DACS: %s\n", buffer, s.c_str());
}

/**
 * Open connection to a dacs server.
 */	
bool MdsOmDacs::openConnection(std::string host, std::string agentName, AuthorizationAgent** pAuthAgent, AuthorizationSystem** pAuthSystem)
{	
	AuthorizationSystem::AUTH_SYSTEM_OPTION_VEC authOptions;

	/* Push back each DACS Sink  host option for each iteration */
	AuthorizationSystem::AUTH_SYSTEM_OPTION_ELEMENT hostOpt, timeoutOpt;
	hostOpt.sOptionName = RFA_String("hosts", 0, false);
	hostOpt.sOptionValue = RFA_String(host.c_str());
	authOptions.push_back(hostOpt);

	/*********************************************************
	* The "connecttimeout" value may need to be increased
	* to ease the load for the library when the connection has
	* a problem such as network congestion, no destionation
	* host etc.
	*
	* The default value for the DACS library is 30 seconds.
	* However, this example sets to 2000 seconds as it is assumed
	* that there are several connections connecting to the
	* DACS system.
	*********************************************************/	
	char buffer[80];
	sprintf(buffer, "%d", waitTime * 1000);
	timeoutOpt.sOptionName = RFA_String("connecttimeout", 0, false);
	timeoutOpt.sOptionValue = RFA_String(buffer, 0, false); // Minimum value  
	authOptions.push_back(timeoutOpt);
					
	try {
		/* Acquire AuthorizationSystem instance for each host */
		*pAuthSystem = AuthorizationSystem::acquire(RFA_String("DACSMC", 0, false), &authOptions);
	} catch (AuthorizationException& ae) {
		log("[" + host + "] Acquire the instance of AuthorizationSystem failed");
		return false;
	}
		
	/* Create AuthorizationAgent instance from acquired AuthorizationSystem. */
	try {
		*pAuthAgent = (**pAuthSystem).createAuthorizationAgent(RFA_String(agentName.c_str()), true);
	} catch(AuthorizationException& ae) {
		log("[" + host + "] create AuthorizationAgent failed");
		return false;
	}
				
	int count = 0;
	int maxCount = 30;
	SLEEP(waitTime * 1000);
	while ((**pAuthAgent).daemonConnectionState() == AuthorizationAgent::authorizationConnectionPending) {
		// TODO add timeout
		SLEEP(1000);
		if (++count > maxCount) {
			log("timeout");			 // TODO
			break;
		}
	}

	/* Check and display connection state. */
	if ((**pAuthAgent).daemonConnectionState() == AuthorizationAgent::authorizationConnectionUp)  {
		log("[" + host + "]  connection state UP");
	} else if ((**pAuthAgent).daemonConnectionState() == AuthorizationAgent::authorizationConnectionDown) { 
		log("[" + host + "]  connection state DOWN");
		return false;
	}

	// Prompt connection is opened.
	log("[" + host + "]  connection opened");

	return true;
}
	
/**
	* Close connection to a dacs server.
	*/
void MdsOmDacs::closeConnection (std::string host, AuthorizationAgent* agent, AuthorizationSystem* system)
{
	if (agent != NULL) {
		// Call destroy() method of AuthorizationAgent to destroy occupied resources.
		agent->destroy();
	}
	if (system != NULL) {
		// Call release() method of AuthorizationSystem to release occupied resources.
		system->release();
	}
	log("[" + host + "] connection closed");
}
	
bool MdsOmDacs::open()
{
	if (hostList.size() == 0) return false;
		
	openMultiConnection();

	// Wait for processing event queue dispatching.
	waitForCallBackEvent(waitTime);
		
	// Check to see if all hosts failed
	if (hostList.size() == 0) return false;
		
	return true;
}
	
bool MdsOmDacs::close()
{
	multiLogout();
	closeMultiConnection();
	return true;
}
	
/**
	* Open connections to the dacs servers.
	*/
void MdsOmDacs::openMultiConnection()
{
	if (hostList.size() == 0)
		return;
		
	// Instatiate event queue instance for keeping callback events.
	dacsEventQueue = EventQueue::create("dacsEventQueue");
				
	vector<std::string>::iterator it = hostList.begin();
	while (it != hostList.end()) {
		const std::string& host = *it;

		// Instantiate AuthorizationSystem instance.
		AuthorizationSystem* system;
		AuthorizationAgent* agent;
		bool bret = openConnection(host, host, &agent, &system);
		if (bret) {
			systems[host] = system;
			agents[host] = agent;
			it++;
		} else {
			log("[" + host + "] connection discarded");
			it = hostList.erase(it);
		}
	}
}
	
/**
	* Close connections to the dacs servers.
	*/
void MdsOmDacs::closeMultiConnection()
{
	// For each host, close its connection.
	vector<std::string>::const_iterator it = hostList.begin();
	while (it != hostList.end()) {
		const std::string& host = *it++;

		AuthorizationSystem* system = NULL;
		map<std::string, AuthorizationSystem*>::iterator itx = systems.find(host);
		if (itx != systems.end()) {
			AuthorizationSystem* system = itx->second;
		}
		AuthorizationAgent* agent = NULL;
		map<std::string, AuthorizationAgent*>::iterator ity = agents.find(host);
		if (ity != agents.end()) {
			agent = ity->second;
		}
		if (system != NULL && agent != NULL) {
			closeConnection(host, agent, system);
		} else {
			// TODO log error
		}
	}
	systems.clear();
	agents.clear();
		
	// Clear allocated resource for event queue.
	if (dacsEventQueue != NULL) {
		dacsEventQueue->destroy();
		dacsEventQueue = NULL;
	}
}
		
/**
 * Log in to the connected dacs server (via the dacs sink host).
 */
Handle* MdsOmDacs::login(MdsOmDacsUser* du, AuthorizationAgent* agent, Client* client)
{		
	/*
	 * Create subsriber's principal instance, the required paramters
	 * of principal is name, position, and application ID (although it is set
	 * by calling setAppName(), but it is applicaiton ID).
	 */
	StandardPrincipalIdentity id;
	id.setName(RFA_String(du->user.c_str()));

	std::string app = "MdsOmDacs";
	id.setAppName(RFA_String(app.c_str()));
	du->application = app;

	char ip[64];
	lookupIPAddress(ip);
	std::string position = ip + string("/net");
	id.setPosition(RFA_String(position.c_str()));
	du->position = position;

	// Encapsulate the principal instance in the AuthorizationRequest instance.
	AuthorizationRequest* principal = AuthorizationRequest::create();
	principal->setPrincipalIdentity(id);
	principal->setUsageLogging(usageLog);

	Handle* handle = NULL;
	try {
		/*
		 * Call AuthorizationAgent instance's login() method to log in the DACS server.
		 * Receive the handle after logging in, but it may be the invalid handle if LOGIN_DENIED
		 * call back event occurs afterward. Using the invalid handle always makes the result of 
		 * access denied for authorization operations in the API.
		 */
		handle = agent->login(*dacsEventQueue, *principal, *client, (void*) du);
	} catch (AuthorizationAgent& ae) {
		log("[" + du->host + "] " + du->user + " log in failed from " + position);
		return NULL;
	}
	log("[" + du->host + "] " + du->user + " logged in from " + position);
	return handle;
}
	
/**
	* Log out from a connected dacs server.
 	*/
void MdsOmDacs::logout(MdsOmDacsUser* du, AuthorizationAgent* agent)
{
	/*
	 * Log out of the DACS server, passing the handle as method parameter.
	 * Using invalid handle parameter does not cause the problem.
	 */
	agent->logout(*du->handle);
	log("[" + du->host + "] " + du->user + " logged out");
}
		
/**
	* Log in to the dacs servers with the same principal.
	*/
void MdsOmDacs::multiLogin(std::string user)
{	
	if (hostList.size() == 0)
		return;

	// Log in subscriber for each host.
	vector<std::string>::const_iterator it = hostList.begin();
	while (it != hostList.end()) {
		const std::string& host = *it++;

		map<std::string, AuthorizationAgent*>::iterator it = agents.find(host);
		if (it != agents.end()) {
			AuthorizationAgent* agent = it->second;
			MdsOmDacsUser* du = new MdsOmDacsUser(host, user);
			Handle* handle = login(du, agent, this);
			if (handle != NULL) {
				du->handle = handle;
				logins[getLoginKey(host, user)] = du;
			}
		}
	}

	waitForCallBackEvent(waitTime);

	// Wait for logins to finish
	int waits = 10;
	int count = 0;
	while (true) {
		bool done = true;
		vector<std::string>::const_iterator it = hostList.begin();
		while (it != hostList.end()) {
			const std::string& host = *it++;
			map<std::string, MdsOmDacsUser*>::iterator it = logins.find(getLoginKey(host, user));
			if (it != logins.end()) {
				MdsOmDacsUser*du = it->second;
				if (du->authDone == false) done = false;
			}
		}
		if (done) break;
		if (++count >= waits) {
			log("failed waiting for logins");
			break;
		}
		waitForCallBackEvent(waitTime);
	}
}
	
/**
	* Log out from the dacs servers.
	*/
void MdsOmDacs::multiLogout()
{
	// Log out for each host.
	map<std::string, MdsOmDacsUser*>::iterator it = logins.begin();
	while (it != logins.end()) {
		MdsOmDacsUser* du = it->second;
		map<std::string, AuthorizationAgent*>::iterator it = agents.find(du->host);
		if (it != agents.end()) {
			AuthorizationAgent* agent = it->second;
			logout(du, agent);
		}
	}
}
	
/**
	* Check subscription permission on a item name.
	* Sometime this subscription checking may be called Subject Based Entitlement (SBE).
	*/
bool MdsOmDacs::checkPermission(bool isSub, MdsOmDacsUser* du, std::string serviceName, std::string itemName, AuthorizationAgent* agent,
								AuthorizationRequest::PerformUsage usage, AuthorizationAgent::AuthorizationRequestType reqtype)
{
	bool isallowed = false;
	AuthorizationCheckStatus authStatus;
		
	/*
		* Create AuthorizationCheckStatus to pass checkSubscription() method
		* as parameter, and receive method result status.
		*/
	try {
		// Check subscription with item name 

		AuthorizationAgent::AuthorizationCheckResult authResult;
		if (isSub) {
			authResult = agent->checkSubscription(*du->handle, usage, authStatus, RFA_String(serviceName.c_str()), RFA_String(itemName.c_str()), 0, NULL, reqtype);
		} else {
			authResult = agent->checkPublication(*du->handle, usage, authStatus, RFA_String(serviceName.c_str()), RFA_String(itemName.c_str()));
		}
			
		// Check result of calling checkSubscription() method
		std::string mode = isSub ? "sub" : "pub";
		if (authResult == AuthorizationAgent::accessAllowed) {
			log("[" + du->host + "] " + du->user + " " + mode + " allowed access to item name " + serviceName + "." + itemName);
			isallowed = true;
		} else {
			log("[" + du->host + "] " + du->user + " " + mode + " denied access to item name " + serviceName + "." + itemName);
			log(string("	Status message: ") + authStatus.getStatusText().c_str());
			isallowed = false;
		}
	} catch (AuthorizationException& ae) {
		log("[" + du->host + "] " + du->user + " subscription to item name " + itemName + " failed");
		log(string("	Status message: ") + authStatus.getStatusText().c_str());
		// log("   Exception: " + ae.tostd::string());
		isallowed = false;
	}
	return isallowed;
}
	
/**
	* This method checks subscription for each user/item a DACS server that is connected by hosts.
	* @throws AuthorizationException
	*/
bool MdsOmDacs::checkSubscribePermission(std::string user, std::string itemName)
{
	pair<std::string, std::string> pair = getSourceAndSymbol(itemName.c_str());
	return checkSubscribePermission(user, pair.first, pair.second);
}
	
bool MdsOmDacs::checkSubscribePermission(std::string user, std::string source, std::string symbol)
{
	MdsOmDacsUser* du = checkLogin(user, source, symbol);
	if (du == NULL) return false;

	map<std::string, AuthorizationAgent*>::iterator it = agents.find(du->host);
	if (it != agents.end()) {
		AuthorizationAgent* agent = it->second;
		return checkPermission(true, du, source, symbol, agent, usageLog, reqType);
	} else {
		return false;
	}
}

/**
	* This method checks subscription for each user/item a DACS server that is connected by hosts.
	* @throws AuthorizationException
	*/
bool MdsOmDacs::checkPublishPermission(std::string user, std::string itemName)
{
	pair<std::string, std::string> pair = getSourceAndSymbol(itemName.c_str());
	return checkPublishPermission(user, pair.first, pair.second);
}
	
bool MdsOmDacs::checkPublishPermission(std::string user, std::string source, std::string symbol)
{
	MdsOmDacsUser* du = checkLogin(user, source, symbol);
	if (du == NULL) return false;

	map<std::string, AuthorizationAgent*>::iterator it = agents.find(du->host);
	if (it != agents.end()) {
		AuthorizationAgent* agent = it->second;
		return checkPermission(false, du, source, symbol, agent, usageLog, reqType);
	} else {
		return false;
	}
}

MdsOmDacsUser* MdsOmDacs::checkLogin(std::string user, std::string source, std::string symbol)
{
	if (user.empty() || source.empty() || symbol.empty()) return NULL;
		 
	std::string host;
	if (useMap) {
		map<std::string, std::string>::iterator it = serviceToHost.find(source);
		if (it == serviceToHost.end()) {
			log("No mapped host to handle " + source);
			return NULL;
		}
	} else {
		host = hostList[hostIndex++];
		if (hostIndex >= hostList.size()) hostIndex = 0;
		if (host.empty()) {
			log("No list host to handle " + source);
			return NULL;
		}
	}
		
	MdsOmDacsUser* du = NULL;
	map<std::string, MdsOmDacsUser*>::iterator it = logins.find(getLoginKey(host, user));
	if (it != logins.end()) {
		du = it->second;
	}
	if (du == NULL) {
		// Need to login first
		multiLogin(user);
	}

	// Search again
	it = logins.find(getLoginKey(host, user));
	if (it != logins.end()) {
		du = it->second;
	}
	if (du == NULL) {
		return NULL;
	}
	return du;
}

void MdsOmDacs::onEntitlementStateChanged(std::string host, int newState)
{
	if (newState == SUBSCRIBER_LOGGEDIN) {
		log("[" + host + "] subscriber has been logged in");
	} else if (newState == SUBSCRIBER_LOGGEDOUT) {
		log("subscriber has been logged out");
		log("[" + host + "] subscriber has been logged out");
	} else {
		log("[" + host + "] unknown log-in state changed");
	}
}

/**
	* A call back event method when DO_REPERMISSION event returns.
	*/
void MdsOmDacs::onDoRepermissionEvent(std::string host, const AuthorizationAgentEvent& eventMsg)
{
	// Only display notify message.
	log("[" + host + "] On DO_REPERMISSION callback event");
}	
	
/**
	* A call back event method when AUTHORIZATION_SYSTEM_NOT_AVAILABLE event returns.
	*/
void MdsOmDacs::onAuthorizationSystemNotAvailableEvent(std::string host, const AuthorizationAgentEvent& eventMsg)
{
	/** 
		* On this call back event, closing the host connection is the only way to do,
		* because the DACS server is unavailable.
		*/
	log("[" + host + "] On AUTHORIZATION_SYSTEM_NOT_AVAILABLE callback event");
}
	
/**
	* A call back event method when LOG_IN_DENIED event returns.
	*/
void MdsOmDacs::onLoginDeniedEvent(std::string host, const AuthorizationAgentEvent& eventMsg) 
{
	log("[" + host + "] On LOGIN_DENIED callback event");
}
	
/**
	* A call back event method when LOG_OUT_FORCED event returns.
	*/
void MdsOmDacs::onLogOutForcedEvent(std::string host, const AuthorizationAgentEvent& eventMsg) 
{
	// Only display notify message.
	log("[" + host + "] On LOG_OUT_FORCED callback event");
}
	
/**
 * Process dispatching routine with the application event queue.
 */
void MdsOmDacs::waitForCallBackEvent(long seconds)
{
	try {
		/*
		 * In order to listen to the event call back from DACS system,
		 * call dispatch() method of event queue instance. Although setting
		 * client for listen up already, the callback method of the client 
		 */
		for (int i = 0; i < seconds; i++) {
			dacsEventQueue->dispatch(1000);
		}
	} catch (...) {
		log("Event Queue Exception");
	}
}
	
pair<std::string, std::string> MdsOmDacs::getSourceAndSymbol(const char* topic)
{
	pair<std::string, std::string> ret;

	if (topic == NULL || *topic == '\0') return ret;

	char* p = strdup(topic);
	char* cp = strchr(p, '.');		// get first '.'
	if (cp != NULL) {
		*cp = '\0';
		ret.first = p;
		ret.second = ++cp;
	}
	free(p);
	return ret;
}

#ifdef WIN32
void MdsOmDacs::lookupIPAddress(char* ipAddress)
{
	char ip[256];
	strcpy(ip, "127.0.0.1");

    char hostname[256];
    int ret = gethostname(hostname, 256);
	if (ret != 0) {
        sprintf(hostname, "localhost");
    }

    struct hostent* host = gethostbyname(hostname);
    if (host != NULL && host->h_addr_list[0] != NULL) {
        char* addrStr = inet_ntoa(*((struct in_addr*)(host->h_addr_list[0])));
		if (addrStr != NULL) {
			strcpy(ip, addrStr);
		}
    }

	strcpy(ipAddress, ip);
}
#else
void MdsOmDacs::lookupIPAddress(char* ipAddress)
{
	char ip[256];
	strcpy(ip, "127.0.0.1");

	struct utsname uts;
    uname(&uts);

    struct hostent* host = gethostbyname(uts.nodename);
    if (host != NULL && host->h_addr_list[0] != NULL) {
        char* addrStr = inet_ntoa(*((struct in_addr*)(host->h_addr_list[0])));
		if (addrStr != NULL) {
			strcpy(ip, addrStr);
		}
    }

	strcpy(ipAddress, ip);
}
#endif

}

