//******************************************************************************
//
// chainname.hxx
// Class which handles chain naming
//
//******************************************************************************

#ifndef MdsOmChainName_H
#define MdsOmChainName_H

#include "MdsOmChains.h"

//******************************************************************************
//
// MDSChainName class
// Controls naming of chains, links in chains and also the elements that
// are returned
//
// NB 'Subject' means fully qualified TIB subject name
//	  'Name'	 means that Market Type and Record Type may be missing
//******************************************************************************
namespace MdsOmNs {

class MdsOmChainName
{
public:
	//--------------------------------------------------------------------------
	// Constructor/destructor
	//--------------------------------------------------------------------------
	MdsOmChainName();
	virtual ~MdsOmChainName();

	//--------------------------------------------------------------------------
	// Initialisor functions
	//--------------------------------------------------------------------------
	MdsOmStatusCode init(const char *source, const char* symbol, ChainConfig *pchainConfig);

	//--------------------------------------------------------------------------
	// Interrogate object
	//--------------------------------------------------------------------------
	const char* getSubject() const { return subject.c_str(); }
	const char* getSymbol() const { return symbol.c_str(); }
	const char* getSource() const { return source.c_str(); }

	//--------------------------------------------------------------------------
	// Return full name of next link subject
	//--------------------------------------------------------------------------
	bool getNextLinkSubject(char *pszNextLinkSubject, int nSize);

protected:
	string source;
	string symbol;
	string subject;

	// pointer to the chain config object
	ChainConfig *_pchainConfig;
};

}

#endif
