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
 *  Name     : CSTBencodedList from STBencodedList.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STBencode.h"

// ================= MEMBER FUNCTIONS =======================


EXPORT_C CSTBencodedList::~CSTBencodedList()
{
	iItems.ResetAndDestroy();
}


EXPORT_C void CSTBencodedList::ToLogL(TInt aIndentation)
{
	CSTBencode::ToLogL(aIndentation);
	//LOG->WriteL(_L8("+LIST"));
	if (iItems.Count() > 0)
	{				
		//LOG->WriteLineL();
		for (TInt i=0; i<iItems.Count(); i++)
		{					
			iItems[i]->ToLogL(aIndentation + 1);
		//	LOG->WriteLineL();
		}
	}
}


EXPORT_C TBencodeType CSTBencodedList::Type()
{
	return EBencodedList;
}


EXPORT_C void CSTBencodedList::AppendL(CSTBencode* aItem)
{
	User::LeaveIfError(iItems.Append(aItem));	
}


EXPORT_C HBufC8* CSTBencodedList::BencodeL() const
{
	CBufFlat* buf = CBufFlat::NewL(128);
	CleanupStack::PushL(buf);
	
	buf->InsertL(0, _L8("l"));
	
	for (TInt i=0; i<iItems.Count(); i++)
	{
		HBufC8* bencodedItem = iItems[i]->BencodeL();
		CleanupStack::PushL(bencodedItem);
		buf->InsertL(buf->Size(), *bencodedItem);
		CleanupStack::PopAndDestroy(); // bencodedItem		
	}
	
	buf->InsertL(buf->Size(), _L8("e"));
	
	HBufC8* bencodedData = buf->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(); // buf	
	
	return bencodedData;
}
