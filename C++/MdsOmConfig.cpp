
#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

MdsOmConfig::MdsOmConfig()
{
	clear();
}

MdsOmConfig::~MdsOmConfig()
{
	transportNames.clear();
}

void MdsOmConfig::clear()
{
	type = MDS_OM_ENV_UNKNOWN;

	transportCb = NULL;
	transportTopicCb = NULL;

	roundRobinTransportCount = 0;
	transportListIndex = 0;

	canThisEnvFailToStart = false;

	loadDictionary = true;

	transportNames.clear();
}

void MdsOmConfig::setType(MdsOmEnvType type)
{
	this->type = type;
}

MdsOmEnvType MdsOmConfig::getType()
{
	return this->type;
}

void MdsOmConfig::setTransportCallback(MdsOmTransportCallback* transportCb)
{
	this->transportCb = transportCb;
}

MdsOmTransportCallback* MdsOmConfig::getTransportCallback()
{
	return this->transportCb;
}

void MdsOmConfig::setBridgeName(string bridgeName)
{
	this->bridgeName = bridgeName;
}

string MdsOmConfig::getBridgeName()
{
	return this->bridgeName;
}

void MdsOmConfig::setTransportName(string transportName)
{
	this->transportName = transportName;
}

string MdsOmConfig::getTransportName()
{
	return this->transportName;
}

void MdsOmConfig::setDictionaryFile(string dictionaryFile)
{
	this->dictionaryFile = dictionaryFile;
}

string MdsOmConfig::getDictionaryFile()
{
	return this->dictionaryFile;
}

void MdsOmConfig::setDictionarySource(string dictionarySource)
{
	this->dictionarySource = dictionarySource;
}

string MdsOmConfig::getDictionarySource()
{
	return this->dictionarySource;
}

void MdsOmConfig::setCanThisEnvFailToStart(bool canThisEnvFailToStart)
{
	this->canThisEnvFailToStart = canThisEnvFailToStart;
}

bool MdsOmConfig::getCanThisEnvFailToStart()
{
	return this->canThisEnvFailToStart;
}

void MdsOmConfig::setLoadDictionary(bool loadDictionary)
{
	this->loadDictionary = loadDictionary;
}

bool MdsOmConfig::getLoadDictionary()
{
	return this->loadDictionary;
}

void MdsOmConfig::setRoundRobinTransport(int roundRobinTransport)
{
	this->roundRobinTransportCount = roundRobinTransport;
}

int MdsOmConfig::getRoundRobinTransport()
{
	return this->roundRobinTransportCount;
}

void MdsOmConfig::setSourcesMap(MdsOmSourcesMap sourcesMap)
{
	this->sourcesMap = sourcesMap;
}

MdsOmSourcesMap MdsOmConfig::getSourcesMap()
{
	return this->sourcesMap;
}

void MdsOmConfig::setTransportNames(MdsOmList<const char*>& transportNames)
{
	transportNames.rewind();
	while (const char* p = transportNames.next()) {
		this->transportNames.add(p);
	}
}

void MdsOmConfig::getTransportNames(MdsOmList<const char*>& transportNames)
{
	this->transportNames.rewind();
	while (const char* p = this->transportNames.next()) {
		transportNames.add(p);
	}
}

void MdsOmConfig::toString()
{
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmConfig: type=%d transportCb=%p roundRobinTransportCount=%d",
		type, transportCb, roundRobinTransportCount);
	mama_log(MAMA_LOG_LEVEL_NORMAL, "MdsOmConfig: transportName=%s dictionaryFile=%s dictionarySource=%s envFail=%d loadDictionary=%d",
		transportName.c_str(), dictionaryFile.c_str(), dictionarySource.c_str(), canThisEnvFailToStart, loadDictionary);
}

}

