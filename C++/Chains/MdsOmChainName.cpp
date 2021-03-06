//******************************************************************************
//
// chainname.cxx
// Implementation of chain name class
//
//******************************************************************************

static const char* fileId = "$Id: MdsOmChainName.cxx 2054 2007-09-26 19:55:21Z f012571 $";

#include "MdsOmChains.h"

#include <ctype.h>

namespace MdsOmNs {
//******************************************************************************
//
// Constructor
//
//******************************************************************************
MdsOmChainName::MdsOmChainName()
{
}

//******************************************************************************
//
// Destructor
//
//******************************************************************************
MdsOmChainName::~MdsOmChainName()
{
}

//******************************************************************************
//
// Initialise chain name object
//
//******************************************************************************
MdsOmStatusCode MdsOmChainName::init(const char *source, const char* symbol, ChainConfig *pchainConfig)
{
	if (!source || !symbol || !pchainConfig) {
		return MDS_OM_STATUS_NULL_ARG;
	}

	// Store the pointer to chain config object for later reference
	_pchainConfig = pchainConfig;
	
	this->source = source;
	this->symbol = symbol;
	this->subject = string(source) + "." + string(symbol);

	return MDS_OM_STATUS_OK;
}

//******************************************************************************
// GetNextLinkName
// Given the name of a link get the next link name.
// This is used when a publisher uses AddElement() and there are more
//	 then 14 elements, and we need to create a new link, and give it a name.
//******************************************************************************
bool MdsOmChainName::getNextLinkSubject(char *pszNextLinkSubject, int nSize)
{
	//----------------------------------------------------------------------------
	// This chain is going to be published, and publish chains have a symbol
	// naming convention as such:
	//	 n#chain_name
	// where 'n' is index of chain(ie 0,1,2...)
	//----------------------------------------------------------------------------
	const char *pszSymbol = getSymbol();
	const char* s = strchr(pszSymbol, '#');
	if (!s) {
		return false;
	}

	// Work out index of current link, ie up to # is numeric
	char szTmp[MAX_STR_TEK_SUBJECT+1];
	int i = 0;
	while (*pszSymbol != '#') {
		char c = *pszSymbol++;
		if (!isdigit(c)) return false;			// must be only number before the #
		szTmp[i++] = c;
	}
	if (i == 0) return false;					// must be at least 1 number before the #
	szTmp[i] = '\0';
	int nLinkIndex = atoi(szTmp) + 1;		// next number for the new link

	// Create symbol of next link(w/o the source)
	snprintf(pszNextLinkSubject, nSize, "%d%s", nLinkIndex, s);

	mama_log(MAMA_LOG_LEVEL_FINE, "MdsOmChainName::GetNextLinkSubject: sym=%s new=%s", getSubject(), pszNextLinkSubject);
	
	return true;
}

}
