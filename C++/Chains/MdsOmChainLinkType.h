
#ifndef MdsOmChainLinkLink_H
#define MdsOmChainLinkLink_H

namespace MdsOmNs {

/**
 * The link types for chains. The current MDSChainSubscribeModel determines the model. Each link in the chain is examined to determine what fields to process.
 */
typedef enum _MdsOmChainLinkType {
	/** Not yet determined. */
	MDSOM_CHAIN_LINK_TYPE_UNKNOWN = 0,

	/** Uses LINK field names. */
	MDSOM_CHAIN_LINK_TYPE_LINK = 1,

	/** Uses LONGLINK field names. */
	MDSOM_CHAIN_LINK_TYPE_LONGLINK = 2
} MdsOmChainLinkType;

}

#endif
