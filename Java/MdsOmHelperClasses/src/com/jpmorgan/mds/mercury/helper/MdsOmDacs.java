
package com.jpmorgan.mds.mercury.helper;

import java.util.*;
import java.util.Map.Entry;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.net.InetAddress;
import java.net.UnknownHostException;

import com.reuters.rfa.common.*;
import com.reuters.rfa.dacs.*;

public class MdsOmDacs implements Client
{
	/**
	 * Constants stand for internal entitlement states
	 */
	protected static final int SUBSCRIBER_UNKNOWN   = 0;
	protected static final int SUBSCRIBER_LOGGEDIN  = 1;
	protected static final int SUBSCRIBER_LOGGEDOUT = 2;

	private long waitTime = 2;
	private DateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

	private boolean useMap = false;		// choose DACS host by source map or round-robin
	private int hostIndex = 0;

	class MdsOmDacsUser
	{
		public MdsOmDacsUser(String host, String user)
		{
			this.user = user;
			this.host = host;
		}
		public String user;
		public String position;
		public String application;
		public Handle handle;
		public String host;
		public boolean authDone = false;
	}

	private String getLoginKey(String host, String user)
	{
		return host + "." + user;
	}
	
	private Map<String, MdsOmDacsUser> logins = new HashMap<String, MdsOmDacsUser>();

	/**
	 * List of DACS Sink hostname.
	 * Each value can be specified as hostname or IP address with port number.
	 * Separate each host value with comma operator (,) character.
	 * Ex.  ddacsv1:8211,ddacsv2:8211
	 */
	private List<String> hostList = new ArrayList<String>();;
	
	/**
	 * List of connected service name for each dacs sink 
	 * that is specified in host list. it is arranged by order
	 * of the connected host.
	 */
	private Map<String, String> serviceToHost = new HashMap<String, String>();
	
	/**
	 * Map between hostname and AuthorizationMCSystem instance.
	 * 
	 * [OPENDACS] AuthorizationMCSystem instance is
	 * used to create AuthorizationAgent instances.
	 */
	private Map<String, AuthorizationMCSystem> systems = new HashMap<String, AuthorizationMCSystem>();
	
	/**
	 * Map between hostname and AuthorizationAgent instance.
	 *
	 * [OPENDACS] This instance is a proxy instance that performs 
	 * authorization operations with the dacs host.
	 * Practically,a single instance of this class can be used by 
	 * many subscribers, only a single instance in an application 
	 * is enough.
	 */
	private Map<String, AuthorizationAgent> agents = new HashMap<String, AuthorizationAgent>();
	
	/**
	 * [OPENDACS] This instance is used to control the level of 
	 * usage logging for authorization operations.
	 * This control,however,is governed under the setting 
	 * usage logging level configuration value of the DACS server. 
	 */	
	private AuthorizationUsageType	usage = AuthorizationUsageType.DONT_PERFORM_USAGE_LOGGING;
	
	/**
	 * [OPENDACS] This instance is used to control the type of 
	 * usage logging for authorization operations.
	 */
	private AuthorizationRequestType reqtype = AuthorizationRequestType.NORMAL_REQUEST_LOGGING;

	/**
	 * [RFA] This instance is used as buffer queue for receiving call back events.
	 * When a response returns to the application it is keep in the event queue.
	 * The application must dispatch event from the queue for processing 
	 * by calling the dispatch() method.The result of calling dispatching will 
	 * execute the processEvent() method of registered client,
	 * when there is any event instance in the queue.
	 */
	private EventQueue dacsEventQueue;
	
	/**
	 * DacsMultiConnectSubscribeClient's constructor
	 */
	public MdsOmDacs()
	{
	}
	
	public boolean addHost(String host)
	{
		if (host == null || host.length() == 0) return false;
		hostList.add(host);
		return true;
	}

	/**
	 * Client's processEvent() implementation, this method is called back
	 * when calling the dispatch() method on the event queue 
	 * and there is at least one Event instance in the event queue. 
	 */	
	public void processEvent(Event event) 
	{
		// Get Host string from registered closure.
		MdsOmDacsUser du = (MdsOmDacsUser) event.getClosure();
		
		// Get handle from event instance.
		// Handle  handle = event.getHandle();
		switch (event.getType()) {
		case Event.AUTHORIZATION_EVENT:
			AuthorizationAgentEventMsg  authEvent = (AuthorizationAgentEventMsg)event;
			AuthorizationAgentEventStatus status = authEvent.getStatus();
			
			// Get user's state from status instance.
			 Status.State state = status.getState();
			 int newEntitlementState = SUBSCRIBER_UNKNOWN;
			 if (state == AuthorizationAgentEventStatus.LOGGED_IN) {
				newEntitlementState = SUBSCRIBER_LOGGEDIN;
			 } else if (state == AuthorizationAgentEventStatus.LOGGED_OUT) {
				newEntitlementState = SUBSCRIBER_LOGGEDOUT;
			 }
			 
			 String str = du.host + "/" + du.user;
			 onEntitlementStateChanged(str, newEntitlementState);

			if (status.getStatusCode() == AuthorizationAgentEventStatus.DO_REPERMISSION) {
				onDoRepermissionEvent(str, authEvent);
			} else if (status.getStatusCode() == AuthorizationAgentEventStatus.AUTHORIZATION_SYSTEM_NOT_AVAILABLE) {
				onAuthorizationSystemNotAvailableEvent(str, authEvent);
			} else if (status.getStatusCode() == AuthorizationAgentEventStatus.LOGIN_DENIED) {
				onLoginDeniedEvent(str, authEvent);
			} else if (status.getStatusCode() == AuthorizationAgentEventStatus.LOGOUT_FORCED) {
				onLogOutForcedEvent(str, authEvent);
			}

			du.authDone = true;
			break;
			
		default:
			break;
		}
	}

	private void log(String s) {
		System.out.println(getNow() + " DACS: " + s);
	}
		
	/**
	 * Open connection to a dacs server.
	 */	
	private AuthorizationAgent openConnection (String host, String agentName, AuthorizationMCSystem system)
	{	
		Properties props = new Properties();
		
		/* In case of multiconnection, the dacs.host property must be specified. */
		props.put("dacs.daemon", host);
		try {
			system.acquire(props);
		} catch(AuthorizationException ae) {
			log("[" + host + "] Set AuthorizationMCSystem properties failed.");
			return null;
		}

		/*
		 * Create AuthorizationAgent instance from newly acquired 
		 * AuthorizationMCSystem instance.
		 */
		AuthorizationAgent agent = system.createAuthorizationAgent(agentName, true);
		
		/* Waiting for startup connection complete */
		while (agent.daemonConnectionState() == AuthorizationConnection.CONNECTION_PENDING) {
			try {
				Thread.sleep(1);
			} catch (InterruptedException ie) {
			}
		}
		
		// Check and display connection state.
		if (agent.daemonConnectionState() == AuthorizationConnection.CONNECTION_UP) {
			log("[" + host + "] connection state is UP");
		} else {
			log("[" + host + "] connection state is DOWN");
			closeConnection(host, agent, system);
			return null;
		}

		log("[" + host + "] connection opened.");
		
		// Return just created AuthorizationAgent instance, null if creation failed.
		return agent;
	}
	
	/**
	 * Close connection to a dacs server.
	 */
	private void closeConnection (String host, AuthorizationAgent agent, AuthorizationMCSystem system)
	{
		if (agent != null) {
			// Call destroy() method of AuthorizationAgent to destroy occupied resources.
			agent.destroy();
		}
		if (system != null) {
			// Call release() method of AuthorizationSystem to release occupied resources.
			system.release();
		}
		log("[" + host + "] connection closed");
	}
	
	public boolean open()
	{
		if (hostList.size() == 0) return false;
		
		openMultiConnection();

		// Wait for processing event queue dispatching.
		waitForCallBackEvent(waitTime);
		
		// Check to see if all hosts failed
		if (hostList.size() == 0) return false;
		
		return true;
	}
	
	public boolean close()
	{
		multiLogout();
		closeMultiConnection();
		return true;
	}
	
	/**
	 * Open connections to the dacs servers.
	 */
	private void openMultiConnection()
	{
		if (hostList.size() == 0)
			return;
		
		// Instatiate event queue instance for keeping callback events.
		dacsEventQueue	= EventQueue.create("dacsEventQueue");
				
		List<String> discardedHostList = new ArrayList<String>();
		for (String host : hostList) {			
			// Instantiate AuthorizationMCSystem instance.
			AuthorizationMCSystem system = new AuthorizationMCSystem();
			AuthorizationAgent agent = openConnection(host, host, system);
			if (agent != null) {
				systems.put(host, system);
				agents.put(host, agent);
			} else {
				log("[" + host + "] connection discarded");
				discardedHostList.add(host);
			}
		}
		
		// Remove all discarded hostnames from the connection list.
		hostList.removeAll(discardedHostList);
	}
	
	/**
	 * Close connections to the dacs servers.
	 */
	private void closeMultiConnection()
	{
		// For each host, close its connection.
		for (String host : hostList) {
			AuthorizationMCSystem system = systems.get(host);
			AuthorizationAgent agent = agents.get(host);
			closeConnection(host, agent, system);

			// Remove host from map stores.
			systems.remove(host);
			agents.remove(host);
		}
		
		// Clear allocated resource for event queue.
		if (dacsEventQueue != null) {
			dacsEventQueue.destroy();
			dacsEventQueue = null;
		}
	}
		
	/**
	 * Log in to the connected dacs server (via the dacs sink host).
	 */
	private Handle login(MdsOmDacsUser du, AuthorizationAgent agent, Client client)
	{		
		/*
		 * Create subsriber's principal instance, the required paramters
		 * of principal is name, position, and application ID (although it is set
		 * by calling setAppName(), but it is applicaiton ID).
		 */
		StandardPrincipalIdentity id = new StandardPrincipalIdentity();
		id.setName(du.user);

		String app = "MdsOmDacs";
		id.setAppName(app);
		du.application = app;

		String ip = "127.0.0.1";
		try {
			InetAddress IP = InetAddress.getLocalHost();
			ip = IP.getHostAddress();
		} catch (UnknownHostException ex) {
			// Inet error, just use localhost for now
		}
		String position = ip + "/net";
		id.setPosition(position);
		du.position = position;

		// Encapsulate the principal instance in the AuthorizationRequest instance.
		AuthorizationRequest principal = new AuthorizationRequest(id);
		Handle handle = null;
		try {
			/*
			 * Call AuthorizationAgent instance's login() method to log in the DACS server.
			 * Receive the handle after logging in, but it may be the invalid handle if LOGIN_DENIED
			 * call back event occurs afterward. Using the invalid handle always makes the result of 
			 * access denied for authorization operations in the API.
			 */
			handle = agent.login(dacsEventQueue, principal, client, du);
		} catch (AuthorizationException ae) {
			log("[" + du.host + "] " + du.user + " log in failed from " + position);
			return null;
		}
		log("[" + du.host + "] " + du.user + " logged in from " + position);
		return handle;
	}
	
	/**
	 * Log out from a connected dacs server.
 	 */
	private void logout (MdsOmDacsUser du, AuthorizationAgent agent)
	{
		/*
		 * Log out of the DACS server, passing the handle as method parameter.
		 * Using invalid handle parameter does not cause the problem.
		 */
		agent.logout(du.handle);
		log("[" + du.host + "] " + du.user + " logged out");
	}
		
	/**
	 * Log in to the dacs servers with the same principal.
	 */
	private void multiLogin(String user)
	{	
		if (hostList.size() == 0)
			return;

		// Log in subscriber for each host.
		for (String host : hostList) {
			AuthorizationAgent agent = agents.get(host);		
			MdsOmDacsUser du = new MdsOmDacsUser(host, user);
			Handle handle = login(du, agent, this);
			if (handle != null) {
				du.handle = handle;
				logins.put(getLoginKey(host, user), du);
			}
		}

		waitForCallBackEvent(waitTime);

		// Wait for logins to finish
		int waits = 10;
		int count = 0;
		while (true) {
			boolean done = true;
			for (String host : hostList) {
				MdsOmDacsUser du = logins.get(getLoginKey(host, user));
				if (du.authDone == false) done = false;
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
	private void multiLogout()
	{
		// Log out for each host.
		for (Entry<String, MdsOmDacsUser> entry : logins.entrySet()) {
			MdsOmDacsUser du = entry.getValue();
			AuthorizationAgent agent = agents.get(du.host);
			logout(du, agent);
		}
	}
	
	/**
	 * Check subscription permission on a item name.
	 * Sometime this subscription checking may be called Subject Based Entitlement (SBE).
	 */
	private boolean checkPermission(boolean isSub, MdsOmDacsUser du, String serviceName, String itemName, AuthorizationAgent agent, AuthorizationUsageType usage, AuthorizationRequestType reqtype)
	{
		AuthorizationCheckStatus authStatus = new AuthorizationCheckStatus();
		boolean isallowed = false;
		
		/*
		 * Create AuthorizationCheckStatus to pass checkSubscription() method
		 * as parameter, and receive method result status.
		 */
		try {
			// Check subscription with item name 

			AuthorizationCheckResult authResult;	
			if (isSub) {
				authResult = agent.checkSubscription(du.handle, usage, reqtype, authStatus, serviceName, itemName);
			} else {
				authResult = agent.checkPublication(du.handle, usage, authStatus, serviceName, itemName);
			}
			
			// Check result of calling checkSubscription() method
			String mode = isSub ? "sub" : "pub";
			if (authResult == AuthorizationCheckResult.ACCESS_ALLOWED) {
				log("[" + du.host + "] " + du.user + " " + mode + " allowed access to item name " + serviceName + "." + itemName);
				isallowed = true;
			} else {
				log("[" + du.host + "] " + du.user + " " + mode + " denied access to item name " + serviceName + "." + itemName);
				log("	Status message: " + authStatus.getStatusText());
				isallowed = false;
			}
		} catch (AuthorizationException ae) {
			log("[" + du.host + "] " + du.user + " subscription to item name " + itemName + " failed");
			log("	Status message: " + authStatus.getStatusText());
			log("   Exception: " + ae.toString());
			isallowed = false;
		}
		return isallowed;
	}
	
	/**
	 * This method checks subscription for each user/item a DACS server that is connected by hosts.
	 * @throws AuthorizationException
	 */
	public boolean checkSubscribePermission(String user, String itemName)
	{
		Pair<String, String> pair = MdsOmUtil.getSourceAndSymbol(itemName);
		return checkSubscribePermission(user, pair.x, pair.y);
	}
	
	public boolean checkSubscribePermission(String user, String source, String symbol)
	{
		MdsOmDacsUser du = checkLogin(user, source, symbol);
		if (du == null) return false;
		AuthorizationAgent agent = agents.get(du.host);
		return checkPermission(true, du, source, symbol, agent, usage, reqtype);
	}

	/**
	 * This method checks subscription for each user/item a DACS server that is connected by hosts.
	 * @throws AuthorizationException
	 */
	public boolean checkPublishPermission(String user, String itemName)
	{
		Pair<String, String> pair = MdsOmUtil.getSourceAndSymbol(itemName);
		return checkPublishPermission(user, pair.x, pair.y);
	}
	
	public boolean checkPublishPermission(String user, String source, String symbol)
	{
		MdsOmDacsUser du = checkLogin(user, source, symbol);
		if (du == null) return false;
		AuthorizationAgent agent = agents.get(du.host);
		return checkPermission(false, du, source, symbol, agent, usage, reqtype);
	}

	private MdsOmDacsUser checkLogin(String user, String source, String symbol)
	{
		if (source == null || symbol == null) return null;
		if (source.length() == 0 || symbol.length() == 0) return null;
		 
		String host = "";
		if (useMap) {
			host = serviceToHost.get(source);
			if (host == null) {
				log("No mapped host to handle " + source);
				return null;
			}
		} else {
			host = hostList.get(hostIndex++);
			if (hostIndex >= hostList.size()) hostIndex = 0;
			if (host == null) {
				log("No list host to handle " + source);
				return null;
			}
		}
		
		MdsOmDacsUser du = logins.get(getLoginKey(host, user));
		if (du == null) {
			// Need to login first
			multiLogin(user);
		}
		du = logins.get(getLoginKey(host, user));
		if (du == null) {
			return null;
		}
		return du;
	}

	public void onEntitlementStateChanged(String host, int newState)
	{
	  if (newState == SUBSCRIBER_LOGGEDIN) {
		 // log("[" + host + "] subscriber has been logged in");
	  } else if (newState == SUBSCRIBER_LOGGEDOUT) {
		 // log("subscriber has been logged out");
		 // log("[" + host + "] subscriber has been logged out");
	  } else {
		 // log("[" + host + "] unknown log-in state changed");
	  }
	}

	/**
	 * A call back event method when DO_REPERMISSION event returns.
	 */
	public void onDoRepermissionEvent(String host, AuthorizationAgentEventMsg eventMsg)
	{
		// Only display notify message.
		log("[" + host + "] On DO_REPERMISSION callback event");
	}	
	
	/**
	 * A call back event method when AUTHORIZATION_SYSTEM_NOT_AVAILABLE event returns.
	 */
	public void onAuthorizationSystemNotAvailableEvent(String host, AuthorizationAgentEventMsg eventMsg)
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
	public void onLoginDeniedEvent(String host, AuthorizationAgentEventMsg eventMsg) 
	{
		log("[" + host + "] On LOGIN_DENIED callback event");
	}
	
	/**
	 * A call back event method when LOG_OUT_FORCED event returns.
	 */
	public void onLogOutForcedEvent(String host, AuthorizationAgentEventMsg eventMsg) 
	{
		// Only display notify message.
		log("[" + host + "] On LOG_OUT_FORCED callback event");
	}
	
	/**
	 * Process dispatching routine with the application event queue.
	 */
	public void waitForCallBackEvent(long seconds)
	{
		try {
			/*
			 * In order to listen to the event call back from DACS system,
			 * call dispatch() method of event queue instance. Although setting
			 * client for listen up already, the callback method of the client 
			 */
			for (int i = 0; i < seconds; i++) {
				dacsEventQueue.dispatch(1000);
			}
		} catch(Exception ex) {
			log("Event Queue Exception - " + ex.getMessage());
		}
	}
	
	private String getNow() {
		return dateFormat.format(new Date());
	}

	private void run()
	{
		String[] items = {"MBPIPE_DEV.AAPL.US/u045594",
						  "IDN_DEV.JPM.N/u045594",
						  "MBPIPE_DEV.TWTR.US/u045594",
						  "MBPIPE_DEV.JPM.US/u045594",
						  "MBPIPE.JPM.US/u045594",
						  "PB-NAMR-DEV.TEST.PUB/u045594",
						  "PB-NAMR.TEST.latency/u045594",
						  "BADSOURCE.TEST.latency/u045594",
						  "LIBERTY.TEST.latency/u045594",
						  "MBPIPE_DEV.JPM.US/nouser",
		};

		try {
			 // String h1 = "ttdnj02mds-5056:8211";
			 // String h2 = "ttdnj02mds-5041:8211";

			String h1 = "ttdnj02mds-5065:8211";
			String h2 = "ttdnj02mds-5066:8211";
			String h3 = "ttdnj02mds-1234:8211";

			addHost(h1);
			addHost(h2);
			addHost(h3);

			// serviceToHost.put("PB-NAMR-DEV", h1);
			// serviceToHost.put("MBPIPE_DEV", h1);
			// serviceToHost.put("IDN_DEV", h1);
			
			// Open connection to each dacs sink host in the list (as specified in command line).
			open();
			
			// Check subscription permission with each dacs server in the list.
			for (String data : items) {
				String tokens[] = data.split("/");
				String itemName = tokens[0];
				String user = tokens[1];
				log("-------------------------------------");
				checkSubscribePermission(user, itemName);
				checkPublishPermission(user, itemName);
			}
			
		} catch (Exception ex) {
			log("DacsMultiConnectSubscribeClient failed: " + ex.toString());
			ex.printStackTrace();
		} finally {
			close();
		}
	}
	
	public static void main(String[] args)
	{
		MdsOmDacs client = new MdsOmDacs();
		client.run();
	}
}

