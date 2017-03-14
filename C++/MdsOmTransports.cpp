#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

// ---------------------------------------------------------
MdsOmTransportSet::MdsOmTransportSet()
{
	listIndex = 0;
}

MdsOmTransportSet::~MdsOmTransportSet()
{
}

// ---------------------------------------------------------
MdsOmTransports::MdsOmTransports()
{
	listIndex = 0;
	env = NULL;
}

void MdsOmTransports::close()
{
	try {
		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::close: %s delete transports map count=%d", env->getName(), transportMap.size());

		MdsOmTransportCollection::const_iterator itx;
		MdsOmTransportMap::const_iterator it;
		for (it = transportMap.begin(); it != transportMap.end(); it++) {
			MdsOmTransportSet* s = it->second;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::close: %s delete transport name=%s", env->getName(), t->getName());
				delete t;
			}
			delete s;
		}
		transportMap.clear();

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::close: %s delete transports list count=%d", env->getName(), transportList.size());
		for (itx = transportList.begin(); itx != transportList.end(); itx++) {
			MdsOmTransport* t = *itx;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::close: %s delete transport name=%s", env->getName(), t->getName());
			delete t;
		}
		transportList.clear();

		mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::close: %s", env->getName());
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmTransports::close: error %s %s", env->getName(), status.toString());
		// Send original exception reference back to caller
		throw;
	}
}

MdsOmTransports::~MdsOmTransports()
{
	//
}

void MdsOmTransports::addTransport(MamaTransport* transport)
{
	// Add to list
	transportList.push_back(new MdsOmTransport(transport));
}

void MdsOmTransports::addTransport(MamaTransport* transport, string source)
{
	// Add to map
	MdsOmTransportSet* s = transportMap[source];
	if (s == NULL) {
		s = new MdsOmTransportSet();
		transportMap[source] = s;
	}
	s->transportList.push_back(new MdsOmTransport(transport));
}

void MdsOmTransports::setStatus(MamaTransport* transport, MdsOmTransportStatus status)
{
	if (isMap()) {
		MdsOmTransportMap::const_iterator it;
		for (it = transportMap.begin(); it != transportMap.end(); it++) {
			MdsOmTransportSet* s = it->second;
			MdsOmTransportCollection::const_iterator itx;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				if (t->getTransport() == transport) {
					t->setStatus(status);
					return;
				}
			}
		}
	} else {
		MdsOmTransportCollection::iterator itx = transportList.begin();
		while (itx != transportList.end()) {
			MdsOmTransport* t = *itx++;
			if (t->getTransport() == transport) {
				t->setStatus(status);
				return;
			}
		}
	}
	mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmTransports::setStatus: did not find transport %s %p", transport->getName(), transport);
	printTransports();
}

void MdsOmTransports::printTransports()
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::printTransports: %s map=%d list=%d", env->getName(), transportMap.size(), transportList.size());
		
	if (isMap()) {
		MdsOmTransportMap::const_iterator it;
		for (it = transportMap.begin(); it != transportMap.end(); it++) {
			MdsOmTransportSet* s = it->second;
			MdsOmTransportCollection::const_iterator itx;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::printTransports: %s %p", t->getTransport()->getName(), t->getTransport());
			}
		}
	} else {
		MdsOmTransportCollection::iterator itx = transportList.begin();
		while (itx != transportList.end()) {
			MdsOmTransport* t = *itx++;
			mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmTransports::printTransports: %s %p", t->getTransport()->getName(), t->getTransport());
		}
	}
}

// Get a transport for a specific source
MamaTransport* MdsOmTransports::getTransport(string source)
{
	if (transportMap.size() == 0 && transportList.size() == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmTransports::getTransport(source): error %s both collections are empty", env->getName());
		throw MdsOmStatus(MDS_OM_STATUS_NO_TRANSPORTS_AVAILABLE);
	}

	if (transportList.size() > 0) {
		return getTransport();
	} else {
		MdsOmTransportMap::const_iterator it = transportMap.find(source);
		if (it != transportMap.end()) {
			// Get a transport from the mapped round-robin list
			MdsOmTransportSet* s = it->second;
			if (s->transportList.size() == 0) {
				// Should not be empty ...
				mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmTransports::getTransport(source): error %s list is empty for map source %s", env->getName(), source.c_str());
				return NULL;
			}
			MdsOmTransport* t = s->transportList.at(s->listIndex);
			if (++s->listIndex >= s->transportList.size()) {
				s->listIndex = 0;
			}
			return t->getTransport();
		} else {
			// Source not found, return a transport from the first list
			MdsOmTransportSet* s = transportMap.begin()->second;
			MdsOmTransport* t = s->transportList.at(0);
			return t->getTransport();
		}
	}
}

MamaTransport* MdsOmTransports::getTransport()
{
	if (transportList.size() == 0) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "MdsOmTransports::getTransport(): error %s list is empty", env->getName());
		throw MdsOmStatus(MDS_OM_STATUS_NO_TRANSPORTS_AVAILABLE);
	}

	MdsOmTransport* t = transportList.at(listIndex);
	if (++listIndex >= transportList.size()) {
		listIndex = 0;
	}
	if (t == NULL) {
		return transportList.at(0)->getTransport();
	} else {
		return t->getTransport();
	}
}

MdsOmTransport* MdsOmTransports::getMdsOmTransport(MamaTransport* transport)
{
	if (transport == NULL) return NULL;

	if (transportList.size() > 0) {
		MdsOmTransportCollection::iterator it = transportList.begin();
		while (it != transportList.end()) {
			MdsOmTransport* t = *it++;
			if (t->getTransport() == transport) return t;
		}
	} else {
		MdsOmTransportMap::const_iterator it = transportMap.begin();
		while (it != transportMap.end()) {
			MdsOmTransportSet* s = it->second;
			MdsOmTransportCollection::const_iterator itx;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				if (t->getTransport() == transport) {
					return t;
				}
			}
			it++;
		}
	}
	return NULL;
}

void MdsOmTransports::getTransports(vector<MamaTransport*>& tlist) const
{
	if (transportList.size() == 0 && transportMap.size() == 0) {
		return;
	}

	if (!isMap()) {
		for (size_t i = 0; i < transportList.size(); i++) {
			tlist.push_back(transportList.at(i)->getTransport());
		}
	} else {
		MdsOmTransportMap::const_iterator it = transportMap.begin();
		while (it != transportMap.end()) {
			MdsOmTransportSet* s = it->second;
			MdsOmTransportCollection::const_iterator itx;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				tlist.push_back(t->getTransport());
			}
			it++;
		}
	}
}

/**
 * Are all transports in connected state?
 */
bool MdsOmTransports::isConnected() const
{
	bool ok = true;
	if (!isMap()) {
		for (size_t i = 0; i < transportList.size(); i++) {
			MdsOmTransport* t = transportList.at(i);
			if (t->isConnected() == false) ok = false;
		}
	} else {
		MdsOmTransportMap::const_iterator it = transportMap.begin();
		while (it != transportMap.end()) {
			MdsOmTransportSet* s = it->second;
			MdsOmTransportCollection::const_iterator itx;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				if (t->isConnected() == false) ok = false;
			}
			it++;
		}
	}
	return ok;
}

/**
 * Is any transport waiting?
 */
bool MdsOmTransports::isConnectWait() const
{
	bool waiting = false;
	if (!isMap()) {
		for (size_t i = 0; i < transportList.size(); i++) {
			MdsOmTransport* t = transportList.at(i);
			if (t->isConnectWait()) waiting = true;
		}
	} else {
		MdsOmTransportMap::const_iterator it = transportMap.begin();
		while (it != transportMap.end()) {
			MdsOmTransportSet* s = it->second;
			MdsOmTransportCollection::const_iterator itx;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				if (t->isConnectWait()) waiting = true;
			}
			it++;
		}
	}
	return waiting;
}

/**
 * Is any transport failed?
 */
bool MdsOmTransports::isConnectFail() const
{
	bool fail = false;
	if (!isMap()) {
		for (size_t i = 0; i < transportList.size(); i++) {
			MdsOmTransport* t = transportList.at(i);
			if (t->isConnectFail()) fail = true;
		}
	} else {
		MdsOmTransportMap::const_iterator it = transportMap.begin();
		while (it != transportMap.end()) {
			MdsOmTransportSet* s = it->second;
			MdsOmTransportCollection::const_iterator itx;
			for (itx = s->transportList.begin(); itx != s->transportList.end(); itx++) {
				MdsOmTransport* t = *itx;
				if (t->isConnectFail()) fail = true;
			}
			it++;
		}
	}
	return fail;
}

}
