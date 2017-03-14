
#include "MdsOmChains.h"

namespace MdsOmNs {

MdsOmElement::MdsOmElement()
{
}

MdsOmElement::~MdsOmElement()
{
}

int MdsOmElement::init(const char *elementName)
{
	 name = elementName;
	 dirty = true;
	 return 0;
}

}
