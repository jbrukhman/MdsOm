
#include "MdsOmChains.h"

namespace MdsOmNs {

MdsOmPubChain::MdsOmPubChain()
{
}

MdsOmPubChain::~MdsOmPubChain()
{
}

MdsOmPubChain* MdsOmPubChain::factory(MdsOm* om, const char* subject, const char* templateName)
{
	return new MdsOmChain(om, subject, templateName);
}

MdsOmPubChain* MdsOmPubChain::factory(MdsOm* om, const char* source, const char* symbol, const char* templateName)
{
	return new MdsOmChain(om, source, symbol, templateName);
}

}

