//******************************************************************************
//******************************************************************************

#include "MdsOmChains.h"
#include "MdsOmChainInternal.h"

//******************************************************************************
//
// ChainConfig class definition
//
//******************************************************************************
namespace MdsOmNs {

ChainConfig::ChainConfig(const char *tName)
{
	templateString = "mama.chains.template.";

	if (tName == NULL || tName[0] == '\0') {
		// No template, look for default specified in config file
		string prop = templateString + "default";
		const char* p = Mama::getProperty(prop.c_str());
		if (p) templateName = p;
	} else {
		// Use their template name
		templateName = tName;
	}
	linkArray = NULL;
	tmpNames = NULL;
	linkNames = NULL;

	nextFieldName = NULL;
	prevFieldName = NULL;
	refCount = NULL;
	prefLink = NULL;

	reversePublish = false;
}

ChainConfig::~ChainConfig()
{
	if (linkArray != NULL) delete [] linkArray;
	if (tmpNames != NULL) free((void*) tmpNames);
	if (linkNames != NULL) free((void*) linkNames);

	if (nextFieldName != NULL) free((void*) nextFieldName);
	if (prevFieldName != NULL) free((void*) prevFieldName);

	if (refCount != NULL) free((void*) refCount);
	if (prefLink != NULL) free((void*) prefLink);
}

MdsOmStatusCode ChainConfig::Init()
{
	// SETUP DEFAULTS, these are used if there is no template available
	// NOTE: These are strdup() here so if the config provides another value we can always free on delete.
	linkNames = strdup("LINK_1,LINK_2,LINK_3,LINK_4,LINK_5,LINK_6,LINK_7,LINK_8,LINK_9,LINK_10,LINK_11,LINK_12,LINK_13,LINK_14");
	nextFieldName = strdup("NEXT_LR");
	prevFieldName = strdup("PREV_LR");
	refCount = strdup("REF_COUNT");
	prefLink = strdup("PREF_LINK");

	mama_log(MAMA_LOG_LEVEL_NORMAL, "ChainConfig::ChainConfig: template=%s", templateName.c_str());

	if (templateName != "") {
		string prop = templateString + templateName + ".LinkNames";

		const char* str = Mama::getProperty(prop.c_str());
		if (str) { 
			free((void*) linkNames); 
			linkNames = strdup(str); 
		}
		mama_log(MAMA_LOG_LEVEL_FINE, "ChainConfig::ChainConfig template[%s] getting LinkNames=%s(%s)", templateName.c_str(), linkNames, prop.c_str());

		if (linkNames[0] == '\0') {
			mama_log(MAMA_LOG_LEVEL_FINE, "ChainConfig::Init: cannot find template=%s or the link name string is empty", templateName.c_str());
			return MDS_OM_STATUS_CHAIN_BAD_FORMAT;
		}

		prop = templateString + templateName + ".NextLinkName";
		str = Mama::getProperty(prop.c_str());
		if (str) { 
			free((void*) nextFieldName); 
			nextFieldName = strdup(str); 
		}
		mama_log(MAMA_LOG_LEVEL_FINE, "ChainConfig::ChainConfig template[%s] getting NextLinkName=%s", templateName.c_str(), nextFieldName);

		prop = templateString + templateName + ".PrevLinkName";
		str = Mama::getProperty(prop.c_str());
		if (str) { 
			free((void*) prevFieldName); 
			prevFieldName = strdup(str); 
		}
		mama_log(MAMA_LOG_LEVEL_FINE, "ChainConfig::ChainConfig template[%s] getting PrevLinkName=%s", templateName.c_str(), prevFieldName);

		prop = templateString + templateName + ".RefCount";
		str = Mama::getProperty(prop.c_str());
		if (str) { 
			free((void*) refCount); 
			refCount = strdup(str); 
		}
		mama_log(MAMA_LOG_LEVEL_FINE, "ChainConfig::ChainConfig template[%s] getting RefCount=%s", templateName.c_str(), refCount); 

		prop = templateString + templateName + ".PrefLink";
		str = Mama::getProperty(prop.c_str());
		if (str) { 
			free((void*) prefLink); 
			prefLink = strdup(str); 
		}
		mama_log(MAMA_LOG_LEVEL_FINE, "ChainConfig::ChainConfig template[%s] getting PrefLink=%s", templateName.c_str(), prefLink);

		prop = templateString + templateName + ".ReversePublish";
		str = Mama::getProperty(prop.c_str());
		if (str) { 
			if (!strcmp(str, "true")) reversePublish = true;
		}
		mama_log(MAMA_LOG_LEVEL_NORMAL, "ChainConfig::ChainConfig template[%s] getting ReversePublish=%s", templateName.c_str(), str); 
	}

	numLinks = 1;
	for (size_t i = 0; i < strlen(linkNames); i++) {
		if (linkNames[i] == ',') numLinks++;
	}

	int indx = 0;
	linkArray = new char*[numLinks];		// freed in destructor
	tmpNames = strdup(linkNames);			// freed in destructor
#ifndef WIN32
	char* last;
#endif
	const char *seps = ",\r\n";
	char* tok = strtok_r(tmpNames, seps, &last);
	while (tok != NULL) {
		tok = trim(tok);				// remove any leading/trailing spaces
		linkArray[indx++] = tok;
		tok = strtok_r(NULL, seps, &last);
	}

	return MDS_OM_STATUS_OK;
}

};
