//
//

#ifndef MDSOMCHAINSUBSCRIBEMODEL_H
#define MDSOMCHAINSUBSCRIBEMODEL_H

namespace MdsOmNs {
/**
 * The subscribe model for incoming chains. This model determines how the links are examined to determine which fields to process.
 * @version $Id$
 * @since 2.3.0.1810
 */
typedef enum _MdsOmChainSubscribeModel {

    /**
     * Not yet determined.
     */
    MDS_CHAIN_SUBSCRIBE_MODEL_UNKNOWN = 0,

    /**
     * Original chain code.
     */
    MDS_CHAIN_SUBSCRIBE_MODEL_V1 = 1,

    /**
     * Kobra model.
     */
    MDS_CHAIN_SUBSCRIBE_MODEL_KOBRA = 2,

    /**
     * SFC model.
     */
    MDS_CHAIN_SUBSCRIBE_MODEL_SFC = 3
} MdsOmChainSubscribeModel;

}
#endif


