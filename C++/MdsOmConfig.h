#ifndef MdsOmConfig_H
#define MdsOmConfig_H

#include "MdsOm.h"

namespace MdsOmNs {

/**
 * Configuration for a single MdsOmEnv.
 *
 * These values are set to configure an env when adding it.
 */
class MDSOMExpDLL MdsOmConfig {
public:
	friend class MdsOmEnv;
	friend class MdsOm;
	friend class MdsOmSubscription;

	MdsOmConfig();

	virtual ~MdsOmConfig();

	void clear();
	void toString();

	void setType(MdsOmEnvType type);
	MdsOmEnvType getType();

	void setTransportCallback(MdsOmTransportCallback* transportCb);
	MdsOmTransportCallback* getTransportCallback();

	void setBridgeName(string bridgeName);
	string getBridgeName();

	void setTransportName(string transportName);
	string getTransportName();

	void setTransportNames(MdsOmList<const char*>& transportNames);
	void getTransportNames(MdsOmList<const char*>& transportNames);

	void setDictionaryFile(string dictionaryFile);
	string getDictionaryFile();

	void setDictionarySource(string dictionarySource);
	string getDictionarySource();

	void setCanThisEnvFailToStart(bool canThisEnvFailToStart);
	bool getCanThisEnvFailToStart();

	void setLoadDictionary(bool loadDictionary);
	bool getLoadDictionary();

	void setRoundRobinTransport(int roundRobinTransport);
	int getRoundRobinTransport();

	void setSourcesMap(MdsOmSourcesMap sourcesMap);
	MdsOmSourcesMap getSourcesMap();

private:
	MdsOmEnvType type;								/**< The environment type. */ 

	MdsOmTransportCallback* transportCb;			/**< The transport callback, defaults to NULL (no callbacks). */

	MdsOmTransportTopicCallback* transportTopicCb;	/**< The transport topic callback, used by user and publisher events */

	string bridgeName;								/**< The bridge name, used if type is UNKNOWN */
	string transportName;							/**< The transport name for the environment bridge. */ 
	string dictionaryFile;							/**< The OpenMama dictionary file name, usually NULL and not used. */ 
	string dictionarySource;						/**< The OpenMama dictionary source name, usually NULL and not used. */

	bool canThisEnvFailToStart;						/**< false (default) = all envs must start OK, true=1 or more envs can fail. Only use this if you really know what you are doing. */

	bool loadDictionary;							/**< true (default) = load dictionary, false = don't load dictionary */

	// Round robin transports
	int roundRobinTransportCount;					/**< The number of transports to create using a round-robin strategy to assign subscriptions to the transports. Default is 0, which is one transport that handles all subscriptions */

	// Source (e.g., PB) transports
	MdsOmSourcesMap sourcesMap;						/**< Map of sources to use per transport. This allows creating multiple transports in this env and directing subscriptions with specific source to a transport. */

	MdsOmList<const char*> transportNames;			/**< List of transport names if the user wants to use different names */

private:
	mutable size_t transportListIndex;
};

}

#endif
