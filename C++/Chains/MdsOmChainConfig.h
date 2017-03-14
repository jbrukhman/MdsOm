//******************************************************************************
//******************************************************************************

#ifndef MdsOmChainConfig_H
#define MdsOmChainConfig_H

#include "MdsOmChains.h"

//******************************************************************************
//
// ChainConfig class
// Holds information Chain link names, number of fields and other important info.
//
//******************************************************************************
namespace MdsOmNs {

class ChainConfig {
public:
	ChainConfig(const char *templateName);
	virtual ~ChainConfig();

	MdsOmStatusCode Init();

	const char* getTemplateName() { return templateName.c_str(); }
	const char* getLinkNames() { return linkNames; }
	char** getLinkArray() { return linkArray; }

	const char* getNextLinkName(){ return nextFieldName; }
	const char* getPrevLinkName(){ return prevFieldName; }
	const char* getRefCount(){ return refCount; }
	const char* getPrefLink() { return prefLink; }

	char* getChainFields(char*);
	char* getAllFields(char*);
	int getAllFieldsLength();
	int getMaxNumLinks() { return numLinks; }

	bool getReversePublish() { return reversePublish; }

private:
	const char* linkNames;
	const char* nextFieldName;
	const char* prevFieldName;
	const char* refCount;
	const char* prefLink;

	std::string templateName;
	std::string templateString;

	char** linkArray;
	char* tmpNames;

	int numLinks;

	bool reversePublish;		/*< Publish links in reverse order - this prevents a not_found race condition */
};

}
#endif
