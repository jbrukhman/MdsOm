
#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

MdsOmAction::MdsOmAction(MdsOmActionCode action)
{
	this->action = action;
}

MdsOmAction::~MdsOmAction(void)
{
}

const char* MdsOmAction::toString() const
{
	static const char* const MdsOmStatusActionNames[] = {
		MDS_OM_1_ENUM(MDS_OM_MAKE_STRINGS)
	};
	static int max = sizeof(MdsOmStatusActionNames) / sizeof(const char*);

	for (int i = 0; i < max; ++i) {
		if (action == (MdsOmActionCode) i) return MdsOmStatusActionNames[i];
	}
	return MdsOmStatusActionNames[MDS_OM_ACTION_UNKNOWN];
}

}