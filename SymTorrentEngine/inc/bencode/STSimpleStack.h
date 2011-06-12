/*****************************************************************************
 * Copyright (C) 2006 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrent
 *
 * SymTorrent is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SymTorrent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/*
 * ============================================================================
 *  Classes  : CSTSimpleStack
 *             CSTSimpleStackBase
 *			  
 *  Part of  : SymTorrent
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STSIMPLESTACK_H
#define SYMTORRENT_STSIMPLESTACK_H

// INCLUDES
#include <e32base.h>
//#include "STDefs.h"


/**
 * CSTSimpleStackBase
 *
 * Base class for the thin templated CSimpleStack. It cannot
 * be instantiated.
 */
class CSTSimpleStackBase : public CBase
{
protected:

	~CSTSimpleStackBase();
	
	CSTSimpleStackBase() { };

protected:

	RPointerArray<CBase> iItems;
};


/**
 * CSTSimpleStack
 *
 * A simple stack with basic functionality. The stack owns the
 * contained items (so it deletes them on destruction).
 */
template <class T>
class CSTSimpleStack : public CSTSimpleStackBase
{
public:

	inline TInt Count() const;
	
	inline void PushL(T* aItem);
	
	/**
	 * @return the top item
	 */
	inline T* Top();
	
	/**
	 * Removes top item.
	 *	 
	 * @return the top item
	 */
	inline T* Pop();
	
	/**
	 * @return the item form the top of the stack at position aIndex
	 */	
	inline T* FromTop(TInt aIndex);
};


// INLINE FUNCTION IMPLEMENTATIONS
template <class T>
inline TInt CSTSimpleStack<T>::Count() const {
	return iItems.Count();	
}

template <class T>
inline void CSTSimpleStack<T>::PushL(T* aItem) {
	User::LeaveIfError(iItems.Append(aItem));	
}

template <class T>
inline T* CSTSimpleStack<T>::Pop() {
	T* item = static_cast<T*>(iItems[iItems.Count() - 1]);
	iItems.Remove(iItems.Count() - 1);
	return item;	
}

template <class T>
inline T* CSTSimpleStack<T>::Top() {	
	return static_cast<T*>(iItems[iItems.Count() - 1]);
}

template <class T>
inline T* CSTSimpleStack<T>::FromTop(TInt aIndex) {
	return static_cast<T*>(iItems[iItems.Count() - (aIndex + 1)]);
}


#endif
