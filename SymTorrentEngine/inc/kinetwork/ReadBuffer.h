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

#ifndef READBUFFER_H
#define READBUFFER_H

#include <e32base.h>

const TUint KReadBrufferReservedSize = 512;
const TUint KReadBrufferGranularity = 16;

/**
 * Dynamic buffer optimized ( :) ) for storing incoming data
 */
class CReadBuffer : public CBase
{
public:

	~CReadBuffer();

	void ConstructL();

	inline void Delete(TInt aPos,TInt aLength);

	inline TPtr8 Ptr() const;

	void AppendL(const TDesC8& aDes);

	inline TInt Size() const;

private:

	CBufFlat* iBuffer;

};

inline TInt CReadBuffer::Size() const {
	return iBuffer->Size();
}

inline TPtr8 CReadBuffer::Ptr() const {
	return iBuffer->Ptr(0);
}

inline void CReadBuffer::Delete(TInt aPos,TInt aLength) {
	iBuffer->Delete(aPos, aLength);
}

#endif
