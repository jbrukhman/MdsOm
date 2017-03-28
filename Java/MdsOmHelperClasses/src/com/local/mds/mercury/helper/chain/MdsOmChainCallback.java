
package com.jpmorgan.mds.mercury.helper.chain;

public interface MdsOmChainCallback {
	/**
	 * Called after all of the links have been successfully retrieved.
	 * @param chain - the chain itself.
	 * @param closure - this was passed in by the app.
	 */
	public void mdsOmChainOnSuccess(MdsOmSubChain chain, Object closure);


	/**
	 * Called for each link as it is returned.
	 * @param chain - the chain itself.
	 * @param link - the link itself.
	 * @param closure - this was passed in by the app.
	 */
	public void mdsOmChainOnLink(MdsOmSubChain chain, MdsOmDataLink link, Object closure);

	/**
	 * Called when a link has an error.
	 * @param chain - the chain itself.
	 * @param link - the link itself.
	 * @param status - the status indicating what the error was.
	 * @param closure - this was passed in by the app.
	 */
	public void mdsOmChainOnError(MdsOmSubChain chain, MdsOmDataLink link, short status, Object closure);
}

