#ifndef MdsOmTransports_H
#define MdsOmTransports_H

#include "MdsOm.h"

namespace MdsOmNs {

enum MdsOmTransportStatus {
	MDS_OM_ENV_STATUS_WAIT,
	MDS_OM_ENV_STATUS_SUCCESS,
	MDS_OM_ENV_STATUS_FAIL
};

/**
 * Wrapper for MamaTransport to track the status.
 */
class MDSOMExpDLL MdsOmTransport  {
public:
	MdsOmTransport(Wombat::MamaTransport* t)
	{
		transport = t;
		connectionStatus = MDS_OM_ENV_STATUS_WAIT;
	}

	Wombat::MamaTransport* getTransport() { return transport; }

	virtual ~MdsOmTransport()
	{
		if (transport != NULL) delete transport;
	}

	const char* getName() const { return transport->getName(); }

	void setStatus(MdsOmTransportStatus status) { connectionStatus = status; }

	/**
	 * Is this env connected?
	 * @return             bool.
	 */
	bool isConnected() const { return connectionStatus == MDS_OM_ENV_STATUS_SUCCESS; }

	/**
	 * Is this env waiting for a connection?
	 * @return             bool.
	 */
	bool isConnectWait() const { return connectionStatus == MDS_OM_ENV_STATUS_WAIT; }

	/**
	 * Is this env failed a connection?
	 * @return             bool.
	 */
	bool isConnectFail() const { return connectionStatus == MDS_OM_ENV_STATUS_FAIL; }

private:
	Wombat::MamaTransport* transport;
	MdsOmTransportStatus connectionStatus;
};

/**
 * This is used for the maps to allow a set of transports to be used
 * for a set of sources, and round-robin across them.
 */
class MDSOMExpDLL MdsOmTransportSet
{
public:
	MdsOmTransportSet();

	~MdsOmTransportSet();

	MdsOmTransportCollection transportList;
	size_t listIndex;
};

/**
 * Manage multiple OpenMAMA transports and their status.
 */
class MDSOMExpDLL MdsOmTransports  {
public:
	MdsOmTransports();

	~MdsOmTransports();

	void close();

	void addTransport(Wombat::MamaTransport* transport);

	void addTransport(Wombat::MamaTransport* transport, string source);

	void setStatus(Wombat::MamaTransport* transport, MdsOmTransportStatus status);

	Wombat::MamaTransport* getTransport(string source);

	Wombat::MamaTransport* getTransport();

	MdsOmTransport* getMdsOmTransport(Wombat::MamaTransport* transport);

	void getTransports(vector<Wombat::MamaTransport*>& tlist) const;

	void printTransports();

	bool isConnected() const;

	bool isConnectWait() const;

	bool isConnectFail() const;

	void setEnv(MdsOmEnv* env) { this->env = env; }

	bool isMap() const { return transportMap.size() > 0; }

	int mapSize() const { return (int) transportMap.size(); }

	int listSize() const { return (int) transportList.size(); }

	MdsOmTransportMap& getMap() { return transportMap; }

private:
	MdsOmEnv* env;
	MdsOmTransportMap transportMap;
	MdsOmTransportCollection transportList;
	size_t listIndex;
};

}

#endif
