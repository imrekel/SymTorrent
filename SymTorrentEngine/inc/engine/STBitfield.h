/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
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

#ifndef SYMTORRENT_STBITFIELD_H
#define SYMTORRENT_STBITFIELD_H

// INCLUDES
#include <e32base.h>
#include "STDefs.h"


/**
 * CSTBitField
 *
 * A fixed length bit field.
 */
class CSTBitField : public CBase
{
public:

	inline TBool IsBitSet(TInt aIndex) const;
	
	void SetBit(TInt aIndex);
	
	void UnsetBit(TInt aIndex);
	
	TBool IsNull() const;
	
	/**
	 * Clones the bitfield. The caller takes ownership of the newly
	 * created object.
	 */
	CSTBitField* CloneL() const;	
	/**
	 * @return the raw bit field
	 */
	inline const TDesC8& Data() const;
	
	void BitwiseNot();
	
	void BitwiseAnd(const CSTBitField* aBitfield);
	
	void BitwiseOr(const CSTBitField* aBitfield);
	
	inline TInt LengthInBytes() const;
	
	void SetL(const TDesC8& aBitField);
	
	inline TBool operator==(const CSTBitField& aBitfield) const;
	
	inline TBool operator!=(const CSTBitField& aBitfield) const;
	
public:

	~CSTBitField();
	
	/**
	 * @param aLength the length of the bitfield in bits
	 */
	void ConstructL(TInt aLength, TBool aSetAllBits = EFalse);
	
	void ConstructL(const TDesC8& aBitField);

protected:

	HBufC8* iBitField;		
};

// INLINE FUNCTION IMPLEMENTATIONS

inline TBool CSTBitField::IsBitSet(TInt aIndex) const {
	return (128u >> (aIndex % 8)) & ((*iBitField)[aIndex/8]);
}

inline const TDesC8& CSTBitField::Data() const {
	return *iBitField;	
}

inline TInt CSTBitField::LengthInBytes() const {
	return iBitField->Length();
}

inline TBool CSTBitField::operator==(const CSTBitField& aBitfield) const {
	/*if (this->LengthInBytes() != aBitfield.LengthInBytes())
		return EFalse;*/
	
	return (*iBitField == (*aBitfield.iBitField));
}

inline TBool CSTBitField::operator!=(const CSTBitField& aBitfield) const {
	return !operator==(aBitfield);
}

#endif
