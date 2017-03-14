#ifndef MdsOmChains_H
#define MdsOmChains_H

#include <MdsOm/MdsOm.h>

#define MAX_STR_TEK_SUBJECT 256

extern MDSOMExpDLL char *trim(char *str);
extern MDSOMExpDLL pair<string, string> getSourceAndSymbol(const char* topic);
extern MDSOMExpDLL vector<string> mds_tokenize(const string& str, const string& delimiters);

namespace MdsOmNs {
	class ChainConfig;
	class MdsOmChain;
	class MdsOmLink;
	class MdsOmElement;
	class MdsOmChainName;
	class MdsOmChainCallback;
	class MdsOmPubChain;
	class MdsOmSubChain;
	class MdsOmDataLink;
}

namespace MdsOmNs {
	typedef list<MdsOmSubscription*> SubscriptionList;
	typedef list<MdsOmLink*> LinkList;
	typedef list<MdsOmElement*> ElementList;
}

#include "MdsOmChainConfig.h"
#include "MdsOmChainName.h"

#include "MdsOmChainApp.h"
#include "MdsOmElement.h"
#include "MdsOmLink.h"

#include "MdsOmPubChain.h"
#include "MdsOmSubChain.h"
#include "MdsOmChain.h"

#endif
