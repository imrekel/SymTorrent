/*****************************************************************************
 * Copyright (C) 2006,2007 Imre Kelényi
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

#ifndef STANNOUNCELIST_H
#define STANNOUNCELIST_H

#include <e32base.h>
#include <badesca.h>

#include "STDefs.h"

class CDesC8ArraySeg;

/**
 * CSTAnnounceList
 */
class CSTAnnounceList : public CBase
{
public:

	/**
	 * Creates an alphabetically sorted list of all addresses.
	 * The caller takes ownership of the list.
	 */
	IMPORT_C CDesC8Array* GetAllAddressesL() const;
	
	/**
	 * @return the total number of addresses stored in the announce list
	 */
	IMPORT_C TInt AddressCount() const;
	
	inline const TPtrC8 ActiveAddress() const;
	
public:

	~CSTAnnounceList();
	
	void Reset();
	
	/**
	 * Adds a new announce address to the specified tier
	 */
	void AddAddressL(TInt aTierIndex, const TDesC8& aAddress);
	
	/**
	 * Shuffles the addresses in each tier
	 */
	void ShuffleL();
		
	/**
	 * Moves the active address to the first place in the tier
	 * (also modifies the active tracker index so it points to
	 * the newly promoted tracker address)
	 */
	void PromoteActiveAddressL();
	
	/**
	 * Modifies the active tier and tracker index so they point
	 * to next tracker in the list.
	 */
	void MoveToNextAddress();
	
	inline const TPtrC8 Address(TInt aTierIndex, TInt aTrackerIndex) const;
	
	/**
	 * Removes empty tiers from the list
	 */
	void RemoveEmptyTiers();

private:

	RPointerArray<CDesC8ArraySeg> iTiers;
	
	TInt iActiveTierIndex;
	
	TInt iActiveTrackerIndex;
};

inline const TPtrC8 CSTAnnounceList::ActiveAddress() const {
	return (*iTiers[iActiveTierIndex])[iActiveTrackerIndex];
}

inline const TPtrC8 CSTAnnounceList::Address(TInt aTierIndex, TInt aTrackerIndex) const {
	return (*iTiers[aTierIndex])[aTrackerIndex];
}

#endif
