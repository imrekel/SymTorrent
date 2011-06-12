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
 *  Name     : CSTBencodedInteger from STBencodedInteger.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 31.01.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STBencode.h"
//#include "STGlobal.h"
#include "STUtil.h"

// ================= MEMBER FUNCTIONS =======================

EXPORT_C CSTBencodedInteger::CSTBencodedInteger(TInt64 aValue)
 : iValue(aValue)
{
}


EXPORT_C void CSTBencodedInteger::ToLogL(TInt aIndentation)
{
	CSTBencode::ToLogL(aIndentation);
//	LOG->WriteL(iValue);		
}


EXPORT_C TBencodeType CSTBencodedInteger::Type()
{
	return EBencodedInteger;
}


EXPORT_C HBufC8* CSTBencodedInteger::BencodeL() const
{		
	HBufC8* buf = HBufC8::NewL(NSTUtils::NumberOfDigits(iValue) + 2);
	TPtr8 bufPtr(buf->Des());
	bufPtr.Num(iValue);
	_LIT8(KLi, "i");
	bufPtr.Insert(0, KLi);
	bufPtr.Append('e');
	
	return buf;
}
