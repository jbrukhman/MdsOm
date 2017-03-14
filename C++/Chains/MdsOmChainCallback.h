#ifndef MdsOmChainCallback_H
#define MdsOmChainCallback_H

namespace MdsOmNs {

/**
 * Interface to receive chain subscribe callbacks on.
 */
class MDSOMExpDLL MdsOmChainCallback
{
public:
	MdsOmChainCallback(void);
	virtual ~MdsOmChainCallback(void);

	/**
	 * Called after all of the links have been successfully retrieved.
	 * @param chain - the chain itself.
	 * @param closure - this was passed in by the app.
	 */
	virtual void mdsOmChainOnSuccess(MdsOmSubChain* chain, void* closure) = 0;


	/**
	 * Called for each link as it is returned.
	 * @param chain - the chain itself.
	 * @param link - the link itself.
	 * @param closure - this was passed in by the app.
	 */
	virtual void mdsOmChainOnLink(MdsOmSubChain* chain, MdsOmDataLink* link, void* closure) = 0;

	/**
	 * Called for each link as it is returned with quality info.
	 * @param chain - the chain itself.
	 * @param link - the link itself.
	 * @param closure - this was passed in by the app.
	 */
	virtual void mdsOmChainOnLinkQuality(MdsOmSubChain* chain, MdsOmDataLink* link, mamaQuality quality, void* closure) {}

	/**
	 * Called when a link has an error.
	 * @param chain - the chain itself.
	 * @param link - the link itself.
	 * @param status - the status indicating what the error was.
	 * @param closure - this was passed in by the app.
	 */
	virtual void mdsOmChainOnError(MdsOmSubChain* chain, MdsOmDataLink* link, MdsOmStatus& status, void* closure) = 0;
};

}

#endif

