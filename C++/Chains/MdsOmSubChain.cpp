
#include "MdsOmChains.h"

namespace MdsOmNs {

MdsOmSubChain::MdsOmSubChain()
{
}

MdsOmSubChain::~MdsOmSubChain()
{
}

MdsOmSubChain* MdsOmSubChain::factory(MdsOm* om, const char* subject)
{
	return new MdsOmChain(om, subject, NULL);
}

MdsOmSubChain* MdsOmSubChain::factory(MdsOm* om, const char* source, const char* symbol)
{
	return new MdsOmChain(om, source, symbol, NULL);
}

}
