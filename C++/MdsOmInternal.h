#ifndef MdsOmInternal_H
#define MdsOmInternal_H

#include "MdsOm.h"

#ifdef WIN32
	#define STRTOK_MDS(x, y, z) strtok_s((x), (y), (z))
#else
	#define STRTOK_MDS(x, y, z) strtok_r((x), (y), (z))
#endif

using namespace std;
using namespace Wombat;

namespace MdsOmNs {
	typedef struct memVals {
		long vsize;
		long rss;
		double memPercent;
	} MdsOmMemVals;

	MDSOMExpDLL char *trim(char *str);
	pair<string, string> getSourceAndSymbol(const char* topic);
	mama_u64_t getNow();
	char* getElapsed(char* buf);
	void mdsOmGetMemVals(int pid, MdsOmMemVals* mv);
	mama_status convertMsgStatusToMamaStatus(mamaMsgStatus msgStatus);
	mamaMsgStatus convertMamaStatusToMsgStatus(mama_status status);

	MamaMsg* newMamaMsg(char payload);
	void deleteMamaMsg(MamaMsg* msg);

	std::string MdsTrimLeft(const std::string& s);
	std::string MdsTrimRight(const std::string& s);
	MDSOMExpDLL std::string MdsTrim(const std::string& s);
}

#endif
