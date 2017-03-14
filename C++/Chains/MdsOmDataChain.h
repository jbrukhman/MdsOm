#ifndef MdsOmDataChain_H
#define MdsOmDataChain_H

#include "MdsOmChainApp.h"

namespace MdsOmNs {

class MDSOMExpDLL MdsOmDataChain
{
public:
	/**
	 * Get a chain with subject (source + symbol) and optional template.
	 * @param om - MdsOm environment.
	 * @param subject - the full subject (source + symbol).
	 * @param templateName - optional template name (may be NULL), found in mama.properties
	 */
	static MdsOmDataChain* Factory(MdsOm* om, const char* subject, const char* templateName);

	/** 
	 * Get a chain with source + symbol and optional template.
	 * @param om - MdsOm environment.
	 * @param source - the source (e.g., PB-NAMR or IDN_RDF).
	 * @param symbol - the symbol.
	 * @param templateName - optional template name (may be NULL), found in mama.properties
	 */
	static MdsOmDataChain* Factory(MdsOm* om, const char* source, const char* symbol, const char* templateName);

	virtual MdsOmStatusCode Destroy() = 0;

	// Subscribe
	virtual MdsOmStatusCode Subscribe(MdsOmChainCallback* cb, void* closure) = 0;
	virtual MdsOmStatusCode Unsubscribe() = 0;

	// Publish
    virtual MdsOmStatusCode Publish() = 0;
    virtual MdsOmStatusCode PublishAll() = 0;
    virtual MdsOmStatusCode Drop() = 0;

	// Only used if an app wants to set additional fields in the links
    virtual MdsOmStatusCode PublishOne() = 0;
    virtual MdsOmStatusCode PublishOneAll() = 0;
    virtual MdsOmStatusCode PublishTwo() = 0;

	// Get all elements
	virtual size_t GetElementCount() const = 0;
	virtual MdsOmList<const char*>* GetElements(MdsOmList<const char*>* l) const = 0;

	// Setup the elements
	virtual MdsOmStatusCode AddElement(const char* elementName) = 0;
    virtual MdsOmStatusCode InsertElement(int indx, const char *pszElementName) = 0;
    virtual MdsOmStatusCode RemoveElement(int indx) = 0;
    virtual MdsOmStatusCode ModifyElement(int indx, const char *pszElementName) = 0;

	virtual MdsOmStatusCode ClearChain() = 0;

	virtual const char* GetChainName() const = 0;
	virtual const char* GetSymbol() const = 0;
	virtual const char* GetSource() const = 0;
	virtual const char* GetSubject() const = 0;

	// Get all link names
	virtual size_t GetLinkCount() const = 0;
	virtual MdsOmList<const char*>* GetLinkNames(MdsOmList<const char*>* l) const = 0;
	virtual MdsOmList<MdsOmDataLink*>* GetLinks(MdsOmList<MdsOmDataLink*>* l) const = 0;

    // Capture 
    virtual MdsOmStatusCode Capture(MdsOmChainCallback* cb, void* closure) = 0;

	// Stats
	// Get the seconds to return the entire chain
	virtual time_t GetTime() const = 0;

	virtual MdsOmStatusCode GetStatusSub() const = 0;

protected:
	MdsOmDataChain();
	virtual ~MdsOmDataChain();
};

}

#endif
