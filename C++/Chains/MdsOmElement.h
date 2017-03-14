
#ifndef MdsOmElement_H
#define MdsOmElement_H

#include "MdsOmChains.h"

namespace MdsOmNs {

//******************************************************************************
// MDSElement class
// Holds information on an individual chain element
//******************************************************************************
class MdsOmElement
{
public:
	friend class MdsOmLink;

	//--------------------------------------------------------------------------
	// Constructors and destructors
	//--------------------------------------------------------------------------
	MdsOmElement();
	virtual ~MdsOmElement();

	//--------------------------------------------------------------------------
	// Initialise object
	//--------------------------------------------------------------------------
	int init(const char *pszName);

	//--------------------------------------------------------------------------
	// Get the element name
	//--------------------------------------------------------------------------
	void setName(const char *pszName) { init(pszName); };
	const char* getName() const { return name.c_str(); };

	void setDirty(bool dirty) { this->dirty = dirty; }
	bool getDirty() { return dirty; }

protected:
	string name;
	bool dirty;
};

}

#endif
