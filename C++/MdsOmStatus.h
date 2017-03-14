#ifndef MdsOmStatus_H
#define MdsOmStatus_H

#include "MdsOm.h"

namespace MdsOmNs {

// ===================================================================
// This macro creates an enum and a method to return a string version of each enum
#define MDS_OM_MAKE_STRINGS(VAR) #VAR,
#define MDS_OM_MAKE_ENUM(VAR) VAR,

#define MDS_OM_1_ENUM(DO) \
	DO(MDS_OM_STATUS_OK) \
    DO(MDS_OM_STATUS_NO_ENVS) \
    DO(MDS_OM_STATUS_INVALID_ENV) \
    DO(MDS_OM_STATUS_BRIDGE_DID_NOT_CONNECT) \
    DO(MDS_OM_STATUS_BRIDGE_TIMEOUT) \
    DO(MDS_OM_STATUS_DICTIONARY_DID_NOT_CONNECT) \
    DO(MDS_OM_STATUS_DICTIONARY_TIMEOUT) \
    DO(MDS_OM_STATUS_DICTIONARY_NO_SOURCE) \
    DO(MDS_OM_STATUS_NULL_ARG) \
    DO(MDS_OM_STATUS_NO_TRANSPORTS_AVAILABLE) \
    DO(MDS_OM_STATUS_TRANSPORT_NOT_PROVIDED) \
    DO(MDS_OM_STATUS_TRANSPORT_NOT_CONNECTED) \
    DO(MDS_OM_STATUS_NO_QUEUES_AVAILABLE) \
    DO(MDS_OM_STATUS_INVALID_CONFIG) \
    DO(MDS_OM_STATUS_CANNOT_FIND_SOURCE_IN_CONFIG) \
    DO(MDS_OM_STATUS_PROPERTIES_FILE_NOT_FOUND) \
	DO(MDS_OM_STATUS_CHAIN_BAD_FORMAT) \
	DO(MDS_OM_STATUS_CHAIN_TEMPLATE_NOT_FOUND) \
	DO(MDS_OM_STATUS_CHAIN_NOT_FOUND) \
	DO(MDS_OM_STATUS_CHAIN_BAD_INDEX) \
	DO(MDS_OM_STATUS_CHAIN_NOT_ENTITLED) \
	DO(MDS_OM_STATUS_CHAIN_ERROR) \
	DO(MDS_OM_STATUS_CHAIN_LINK_DELETE) \
	DO(MDS_OM_STATUS_CHAIN_LINK_TIMEOUT) \
	DO(MDS_OM_STATUS_UNKNOWN)
typedef enum MDSOMExpDLL _MdsOmStatusCode {
    MDS_OM_1_ENUM(MDS_OM_MAKE_ENUM)
} MdsOmStatusCode;

/**
  * Status class thrown by MdsOm.
  */
class MDSOMExpDLL MdsOmStatus {
public:
    MdsOmStatus(MdsOmStatusCode status);

	virtual ~MdsOmStatus (void);

	/**
	 * Get a string version of the status.
	 * @return             string.
	 * @throws             MamaStatus, MdsOmStatus
	 */
    const char* toString(void) const;

	/**
	 * Get the status.
	 * @return             MdsOmStatusCode.
	 * @throws             MamaStatus, MdsOmStatus
	 */
    MdsOmStatusCode getStatus() const { return status; }

private:
	MdsOmStatusCode status;
};

}

#endif
