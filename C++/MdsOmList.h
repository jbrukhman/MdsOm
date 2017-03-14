/** 
 * @file MdsOmList.h
 * @brief Single include file for MdsOm.
 * @see <twiki>
 */

#ifndef MdsOmList_H
#define MdsOmList_H

#include "MdsOm.h"

namespace MdsOmNs {

/**
 * This class is used to return lists of elements or links.
 * This is to deal with Windows DLL issues with passing memory or STLs across
 * DLL boundaries when different versions of Visual Studio are used.
 * http://stackoverflow.com/questions/5661738/how-can-i-use-standard-library-stl-classes-in-my-dll-interface-or-abi/5664491#5664491
 */
template <class T>
class MDSOMExpDLL MdsOmList
{
public:
	MdsOmList();

	virtual ~MdsOmList();

	/**
	 * Destroy(delete) the list.
	 * This is used rather than the destructor to prevent memory issues between DLLs in Windows.
	 */
	void destroy();

	/**
	 * Clear all of the elements from the list.
	 */
	void clear();

	/**
	 * Get the next element from the list.
	 * @returns NULL if no more elements.
	 */
	T next();

	/**
	 * Rewind the list.
	 */
	void rewind();

	/**
	 * Return the size of the list.
	 */
	size_t size();

	/**
	 * Add an item.
	 */
	void add(T val);

private:
	size_t index;
	std::vector<T> data;
};

}

#endif
