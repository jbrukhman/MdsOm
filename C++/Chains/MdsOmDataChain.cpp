
#include "MdsOmChains.h"

namespace MdsOmNs {

MdsOmDataChain::MdsOmDataChain()
{
}

MdsOmDataChain::~MdsOmDataChain()
{
}

MdsOmDataChain* MdsOmDataChain::Factory(MdsOm* om, const char* subject, const char* templateName)
{
	return new MdsOmChain(om, subject, templateName);
}

MdsOmDataChain* MdsOmDataChain::Factory(MdsOm* om, const char* source, const char* symbol, const char* templateName)
{
	return new MdsOmChain(om, source, symbol, templateName);
}

}
