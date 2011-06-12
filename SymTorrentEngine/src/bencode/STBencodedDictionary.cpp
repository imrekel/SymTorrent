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
 *  Name     : CSTBencodedDictionary from STBencodedDictionary.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STBencode.h"

// ================= MEMBER FUNCTIONS =======================

EXPORT_C CSTBencodedDictionary::~CSTBencodedDictionary()
{
	iEntries.ResetAndDestroy();
}


EXPORT_C TBencodeType CSTBencodedDictionary::Type()
{
	return EBencodedDictionary;
}


EXPORT_C void CSTBencodedDictionary::AddEntryL(CSTBencodedDictionaryEntry* aEntry)
{	
	User::LeaveIfError(
		iEntries.InsertInOrder(aEntry, 
		TLinearOrder<CSTBencodedDictionaryEntry>(CSTBencodedDictionaryEntry::Compare)));
}

	
EXPORT_C void CSTBencodedDictionary::AddEntryL(CSTBencodedString* aKey, CSTBencode* aValue)
{
	CleanupStack::PushL(aKey);
	CleanupStack::PushL(aValue);
	
	CSTBencodedDictionaryEntry* entry = 
		new (ELeave) CSTBencodedDictionaryEntry(aKey, aValue);
		
	CleanupStack::Pop(); // aValue
	CleanupStack::Pop(); // aKey
		
	CleanupStack::PushL(entry);
	AddEntryL(entry);
	CleanupStack::Pop();	
}


EXPORT_C void CSTBencodedDictionary::AddEntryL(const TDesC8& aKey, CSTBencode* aValue)
{
	CleanupStack::PushL(aValue);
	
	CSTBencodedString* key = CSTBencodedString::NewL(aKey);
	
	CleanupStack::Pop(); // aValue
	
	AddEntryL(key, aValue);
}


EXPORT_C CSTBencode* CSTBencodedDictionary::EntryValue(const TDesC8& aKey)
{
	for (TInt i=0; i<iEntries.Count(); i++)
	{
		if (iEntries[i]->Key()->Value() == aKey)
			return iEntries[i]->Value();
	}
	
	return NULL;
}

	
EXPORT_C CSTBencode* CSTBencodedDictionary::EntryValue(CSTBencodedString* aKey)
{
	return EntryValue(aKey->Value());
}


EXPORT_C void CSTBencodedDictionary::ToLogL(TInt aIndentation)
{
	CSTBencode::ToLogL(aIndentation);
	//LOG->WriteL(_L8("+DICTIONARY"));
	for (TInt i=0; i<iEntries.Count(); i++)
	{
		//LOG->WriteLineL();
	/*	CSTBencodedString* key = 
			const_cast<CSTBencodedString*>(iEntries[i]->Key());*/
		
		iEntries[i]->Key()->ToLogL(aIndentation + 1);
	//	LOG->WriteL(_L8(" :"));
		if (iEntries[i]->Key()->Value() != _L8("pieces"))
		{
			iEntries[i]->Value()->ToLogL(aIndentation + 2);
		}
	//	else
	//		LOG->WriteL(_L8("[pieces]"));
	//	LOG->WriteLineL();
		
	}
}


EXPORT_C HBufC8* CSTBencodedDictionary::BencodeL() const
{
	CBufFlat* buf = CBufFlat::NewL(128);
	CleanupStack::PushL(buf);
	
	buf->InsertL(0, _L8("d"));
	
	for (TInt i=0; i<iEntries.Count(); i++)
	{
		CSTBencodedDictionaryEntry* entry = iEntries[i];
		HBufC8* bencodedKey = entry->Key()->BencodeL();
		CleanupStack::PushL(bencodedKey);
		buf->InsertL(buf->Size(), *bencodedKey);
		CleanupStack::PopAndDestroy(); // bencodedKey
		
		HBufC8* bencodedItem = entry->Value()->BencodeL();
		CleanupStack::PushL(bencodedItem);
		buf->InsertL(buf->Size(), *bencodedItem);
		CleanupStack::PopAndDestroy(); // bencodedItem		
	}
	
	buf->InsertL(buf->Size(), _L8("e"));
	
	HBufC8* bencodedData = buf->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(); // buf	
	
	return bencodedData;
}
