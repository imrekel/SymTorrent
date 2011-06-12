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
 *  Name     : CSTBitField from STBitField.cpp
 *  Part of  : SymTorrent
 *  Created  : 21.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STBitField.h"

// ================= MEMBER FUNCTIONS =======================

CSTBitField::~CSTBitField()
{
	delete iBitField;
}

void CSTBitField::ConstructL(TInt aLength, TBool aSetAllBits)
{
	TInt lengthInBytes = aLength / 8;
	
	if (aLength % 8)
		lengthInBytes++;
	
	iBitField = HBufC8::NewMaxL(lengthInBytes);
	TPtr8 bitFieldPtr(iBitField->Des());
	
	TChar value;
	if (aSetAllBits)
		value = 0xFF;
	else
		value = 0;
	
	for (TInt i=0; i<bitFieldPtr.Length(); i++)
		bitFieldPtr[i] = value;
	
	// Set the extra bits of the last byte to 0
	if ((aLength % 8) && aSetAllBits)
	{
		bitFieldPtr[lengthInBytes - 1] = 0;
		for (TInt i=0; i<(aLength%8); i++)
			SetBit((lengthInBytes - 1)*8 + i);
	}
}

void CSTBitField::ConstructL(const TDesC8& aBitField)
{
	iBitField = aBitField.AllocL();
}

void CSTBitField::SetBit(TInt aIndex) 
{
	TPtr8 bitFieldPtr(iBitField->Des());
	bitFieldPtr[aIndex / 8] |= (128u >> (aIndex % 8));	
}


void CSTBitField::UnsetBit(TInt aIndex) 
{
	TPtr8 bitFieldPtr(iBitField->Des());
	bitFieldPtr[aIndex / 8] &= (~(128u >> (aIndex % 8)));	
}


TBool CSTBitField::IsNull() const
{
	for (TInt i=0; i<iBitField->Length(); i++)
		if ((*iBitField)[i] != 0)
			return EFalse;
		
	return ETrue;
}


CSTBitField* CSTBitField::CloneL() const
{
	CSTBitField* clone = new (ELeave) CSTBitField;
	clone->ConstructL(iBitField->Length() * 8);
	
	delete clone->iBitField;
	clone->iBitField = NULL;
	clone->iBitField = (*iBitField).AllocL();
	
	return clone;	
}


void CSTBitField::BitwiseNot()
{
	TPtr8 bitfieldPtr(iBitField->Des());
	for (TInt i=0; i<iBitField->Length(); i++)
		bitfieldPtr[i] = ~(bitfieldPtr[i]);
}


void CSTBitField::BitwiseAnd(const CSTBitField* aBitField)
{
	TPtr8 bitfieldPtr(iBitField->Des());
	for (TInt i=0; i<iBitField->Length(); i++)
		bitfieldPtr[i] = bitfieldPtr[i] & aBitField->Data()[i];
	
}

void CSTBitField::BitwiseOr(const CSTBitField* aBitField)
{
	TPtr8 bitfieldPtr(iBitField->Des());
	for (TInt i=0; i<iBitField->Length(); i++)
		bitfieldPtr[i] = bitfieldPtr[i] | aBitField->Data()[i];
	
}

void CSTBitField::SetL(const TDesC8& aBitField)
{
	delete iBitField;
	iBitField = NULL;
	
	iBitField = aBitField.AllocL();
}

