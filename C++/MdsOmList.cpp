/** 
 * @file MdsOmList.h
 * @brief Single include file for MdsOm.
 * @see <twiki>
 *
 * This is to deal with Windows DLL issues with passing memory or STLs across
 * DLL boundaries when different versions of Visual Studio are used.
 */

#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

template <class T>
MdsOmList<T>::MdsOmList()
{
	clear();
}

template <class T>
MdsOmList<T>::~MdsOmList()
{
}

template <class T>
void MdsOmList<T>::destroy()
{
	delete this;
}

template <class T>
T MdsOmList<T>::next()
{
	if (index >= data.size()) return NULL;
	return data[index++];
}

template <class T>
void MdsOmList<T>::add(T val)
{
	if (val) data.push_back(val);
}

template <class T>
void MdsOmList<T>::clear()
{
	index = 0;
	data.clear();
}

template <class T>
void MdsOmList<T>::rewind()
{
	index = 0;
}

template <class T>
size_t MdsOmList<T>::size()
{
	return data.size();
}

template class MdsOmList<const char*>;
template class MdsOmList<MdsOmDataLink*>;

}
