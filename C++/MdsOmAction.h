#ifndef MdsOmAction_H
#define MdsOmAction_H

#include "MdsOm.h"

namespace MdsOmNs {

// ===================================================================
// This macro creates an enum and a method to return a string version of each enum
#define MDS_OM_MAKE_STRINGS(VAR) #VAR,
#define MDS_OM_MAKE_ENUM(VAR) VAR,

#define MDS_OM_2_ENUM(DO) \
	DO(MDS_OM_ACTION_OK) \
    DO(MDS_OM_ACTION_CANCEL) \
	DO(MDS_OM_ACTION_USER) \
	DO(MDS_OM_ACTION_UNKNOWN)

typedef enum MDSOMExpDLL _MdsOmActionCode {
    MDS_OM_2_ENUM(MDS_OM_MAKE_ENUM)
} MdsOmActionCode;

/**
  * Action class thrown by MdsOm.
  */
class MDSOMExpDLL MdsOmAction {
public:
    MdsOmAction(MdsOmActionCode action);

    ~MdsOmAction (void);

	/**
	 * Get a string version of the action.
	 * @return             string.
	 * @throws             MamaAction, MdsOmAction
	 */
    const char* toString(void) const;

	/**
	 * Get the action.
	 * @return             MdsOmActionCode.
	 * @throws             MamaAction, MdsOmAction
	 */
    MdsOmActionCode getAction() const { return action; }

private:
	MdsOmActionCode action;
};

}

#endif
