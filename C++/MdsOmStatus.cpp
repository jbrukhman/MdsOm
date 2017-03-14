
#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

MdsOmStatus::MdsOmStatus(MdsOmStatusCode status)
{
	this->status = status;
}

MdsOmStatus::~MdsOmStatus(void)
{
}

const char* MdsOmStatus::toString() const
{
	static const char* const MdsOmStatusCodeNames[] = {
		MDS_OM_1_ENUM(MDS_OM_MAKE_STRINGS)
	};
	static int max = sizeof(MdsOmStatusCodeNames) / sizeof(const char*);

	for (int i = 0; i < max; ++i) {
		if (status == (MdsOmStatusCode) i) return MdsOmStatusCodeNames[i];
	}
	return MdsOmStatusCodeNames[MDS_OM_STATUS_UNKNOWN];
}

}

