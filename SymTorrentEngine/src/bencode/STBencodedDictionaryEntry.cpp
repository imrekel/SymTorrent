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

/* ============================================================================
 *  Name     : CSTBencodedDictionaryEntry from STBencodedDictionaryEntry.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STBencode.h"

// ================= MEMBER FUNCTIONS =======================

EXPORT_C CSTBencodedDictionaryEntry::CSTBencodedDictionaryEntry(	CSTBencodedString* aKey,
														CSTBencode*	aValue)
 : iKey(aKey), iValue(aValue)
{
}


EXPORT_C CSTBencodedDictionaryEntry::~CSTBencodedDictionaryEntry()
{
	delete iKey;
	delete iValue;
}


EXPORT_C TInt CSTBencodedDictionaryEntry::Compare(
	const CSTBencodedDictionaryEntry& aFirst, 
	const CSTBencodedDictionaryEntry& aSecond)
{
	CSTBencodedDictionaryEntry& first = 
		const_cast<CSTBencodedDictionaryEntry&>(aFirst);
	CSTBencodedDictionaryEntry& second = 
		const_cast<CSTBencodedDictionaryEntry&>(aSecond);
	
	if (first.Key()->Value() == second.Key()->Value())
		return 0;

	if (first.Key()->Value() < second.Key()->Value())
		return -1;
	else
		return 1;
}
