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
 *  Name     : CSTBencodedString from STBencodedString.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STBencode.h"
#include "STUtil.h"

// ================= MEMBER FUNCTIONS =======================

EXPORT_C CSTBencodedString* CSTBencodedString::NewL(const TDesC8& aDes)
{
	CSTBencodedString* instance = CSTBencodedString::NewLC(aDes);
	CleanupStack::Pop();	
	
	return instance;
}
	
	
EXPORT_C CSTBencodedString* CSTBencodedString::NewLC(const TDesC8& aDes)
{
	CSTBencodedString* instance = new (ELeave) CSTBencodedString;
	CleanupStack::PushL(instance);
	instance->ConstructL(aDes);
	
	return instance;	
}	

	
void CSTBencodedString::ConstructL(const TDesC8& aDes)
{
	SetValueL(aDes);
}


EXPORT_C CSTBencodedString::~CSTBencodedString()
{
	delete iBuffer;
}


EXPORT_C void CSTBencodedString::ToLogL(TInt aIndentation)
{
	CSTBencode::ToLogL(aIndentation);
//	LOG->WriteL(*iBuffer);
}



EXPORT_C TBencodeType CSTBencodedString::Type()
{
	return EBencodedString;
}


EXPORT_C void CSTBencodedString::SetValueL(const TDesC8& aDes)
{
	delete iBuffer;
	iBuffer = NULL;
	
	iBuffer = aDes.AllocL();
}


EXPORT_C HBufC8* CSTBencodedString::BencodeL() const
{
	TInt stringLength = iBuffer->Length();	
	HBufC8* buf = HBufC8::NewL(stringLength + 
		NSTUtils::NumberOfDigits(stringLength) + 1);
	TPtr8 bufPtr(buf->Des());
	bufPtr.Num(stringLength);
	bufPtr.Append(':');
	bufPtr.Append(*iBuffer);	
	return buf;
}
