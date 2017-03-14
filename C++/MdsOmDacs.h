
#ifndef MDSOMDACS_H
#define MDSOMDACS_H

#include <vector>
#include <string>
#include <map>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <Common/StandardPrincipalIdentity.h>
#include <Common/Handle.h>
#include <Common/Client.h>
#include <Common/Event.h>
#include <Common/EventQueue.h>
#include <Common/RFA_String.h>
#include <Common/RFA_Vector.h>

#include <Dacs/AuthCheckStatus.h>
#include <Dacs/AuthException.h>
#include <Dacs/AuthLock.h>
#include <Dacs/AuthLockData.h>
#include <Dacs/AuthSystem.h>
#include <Dacs/AuthAgent.h>
#include <Dacs/AuthAgentEvent.h>
#include <Dacs/AuthRequest.h>
#include <Dacs/AuthLockData.h>

#ifdef WIN32
	#include <windows.h>
	#define     SLEEP(X)    Sleep(X)
#else
	#include <unistd.h>
	#include <netdb.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/utsname.h>
	#define     SLEEP(X)    usleep(X*1000)
#endif

namespace MdsOm {

using namespace std;
using namespace rfa::common;
using namespace rfa::dacs;

class MdsOmDacsUser
{
public:
	MdsOmDacsUser(std::string host, std::string user)
	{
		this->user = user;
		this->host = host;
		authDone = false;
		handle = NULL;
	}
	std::string user;
	std::string position;
	std::string application;
	Handle* handle;
	std::string host;
	bool authDone;
};

class MdsOmDacs : public rfa::common::Client
{
public:
	enum DacsSubscriberState {
		/** Constants stand for internal entitlement states */
		SUBSCRIBER_UNKNOWN   = 0,
		SUBSCRIBER_LOGGEDIN  = 1,
		SUBSCRIBER_LOGGEDOUT = 2
	};

	MdsOmDacs();

	~MdsOmDacs();
	
	bool addHost(std::string host);
	
	bool open();
	
	bool close();	
	
	/**
	 * This method checks subscription for each user/item a DACS server that is connected by hosts.
	 * @throws AuthorizationException
	 */
	bool checkSubscribePermission(std::string user, std::string itemName);
	
	bool checkSubscribePermission(std::string user, std::string source, std::string symbol);

	/**
	 * This method checks publish for each user/item a DACS server that is connected by hosts.
	 * @throws AuthorizationException
	 */
	bool checkPublishPermission(std::string user, std::string itemName);
	
	bool checkPublishPermission(std::string user, std::string source, std::string symbol);

private:
	long waitTime;
	bool useMap;		// choose DACS host by source map or round-robin
	int hostIndex;

	std::string getLoginKey(std::string host, std::string user);

	map<std::string, MdsOmDacsUser*> logins;

	/**
	 * List of DACS Sink hostname.
	 * Each value can be specified as hostname or IP address with port number.
	 * Separate each host value with comma operator (,) character.
	 * Ex.  ddacsv1:8211,ddacsv2:8211
	 */
	vector<std::string> hostList;
	
	/**
	 * List of connected service name for each dacs sink 
	 * that is specified in host list. it is arranged by order
	 * of the connected host.
	 */
	map<std::string, std::string> serviceToHost;
	
	/**
	 * Map between hostname and AuthorizationMCSystem instance.
	 * 
	 * [OPENDACS] AuthorizationMCSystem instance is
	 * used to create AuthorizationAgent instances.
	 */
	map<std::string, AuthorizationSystem*> systems;
	
	/**
	 * Map between hostname and AuthorizationAgent instance.
	 *
	 * [OPENDACS] This instance is a proxy instance that performs 
	 * authorization operations with the dacs host.
	 * Practically,a single instance of this class can be used by 
	 * many subscribers, only a single instance in an application 
	 * is enough.
	 */
	map<std::string, AuthorizationAgent*> agents;
	
	AuthorizationRequest::PerformUsage usageLog;
    AuthorizationAgent::AuthorizationRequestType reqType;

	/**
	 * [RFA] This instance is used as buffer queue for receiving call back events.
	 * When a response returns to the application it is keep in the event queue.
	 * The application must dispatch event from the queue for processing 
	 * by calling the dispatch() method.The result of calling dispatching will 
	 * execute the processEvent() method of registered client,
	 * when there is any event instance in the queue.
	 */
	EventQueue* dacsEventQueue;

	bool openConnection(std::string host, std::string agentName, AuthorizationAgent** pAuthAgent, AuthorizationSystem** pAuthSystem);
	void closeConnection(std::string host, AuthorizationAgent* agent, AuthorizationSystem* system);

	/**
	 * Client's processEvent() implementation, this method is called back
	 * when calling the dispatch() method on the event queue 
	 * and there is at least one Event instance in the event queue. 
	 */	
	void processEvent(const Event& event);

	void log(std::string s);
		
	void openMultiConnection();

	void closeMultiConnection();

	/**
	 * Log in to the connected dacs server (via the dacs sink host).
	 */
	Handle* login(MdsOmDacsUser* du, AuthorizationAgent* agent, Client* client);
	
	/**
	 * Log out from a connected dacs server.
 	 */
	void logout(MdsOmDacsUser* du, AuthorizationAgent* agent);
		
	/**
	 * Log in to the dacs servers with the same principal.
	 */
	void multiLogin(std::string user);
	
	/**
	 * Log out from the dacs servers.
	 */
	void multiLogout();
	
	/**
	 * Check subscription permission on a item name.
	 * Sometime this subscription checking may be called Subject Based Entitlement (SBE).
	 */
	bool checkPermission(bool isSub, MdsOmDacsUser* du, std::string serviceName, std::string itemName, AuthorizationAgent* agent,
		AuthorizationRequest::PerformUsage usage, AuthorizationAgent::AuthorizationRequestType reqtype);
	
	MdsOmDacsUser* checkLogin(std::string user, std::string source, std::string symbol);

	void run();

	pair<std::string, std::string> getSourceAndSymbol(const char* topic);

	void lookupIPAddress(char* ipAddress);

	/** OpenDacs callbacks */
	void onEntitlementStateChanged(std::string host, int newState);

	/**
	 * A call back event method when DO_REPERMISSION event returns.
	 */
	void onDoRepermissionEvent(std::string host, const AuthorizationAgentEvent& eventMsg);
	
	/**
	 * A call back event method when AUTHORIZATION_SYSTEM_NOT_AVAILABLE event returns.
	 */
	void onAuthorizationSystemNotAvailableEvent(std::string host, const AuthorizationAgentEvent& eventMsg);
	
	/**
	 * A call back event method when LOG_IN_DENIED event returns.
	 */
	void onLoginDeniedEvent(std::string host, const AuthorizationAgentEvent& eventMsg);
	
	/**
	 * A call back event method when LOG_OUT_FORCED event returns.
	 */
	void onLogOutForcedEvent(std::string host, const AuthorizationAgentEvent& eventMsg);
	
	/**
	 * Process dispatching routine with the application event queue.
	 */
	void waitForCallBackEvent(long seconds);
};

}

#endif
