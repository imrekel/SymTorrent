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

#ifndef WRITEBUFFER_H
#define WRITEBUFFER_H

#include <e32base.h>

const TInt KDefaultWriteBrufferGranularity = 256;

class CWriteBuffer : public CBase
{
public:

	static CWriteBuffer* NewLC(TInt aGranularity = KDefaultWriteBrufferGranularity);
	
	static CWriteBuffer* NewL(TInt aGranularity = KDefaultWriteBrufferGranularity);

	~CWriteBuffer();

	/**
	 * Reads data from the buffer. The processed (read) data is deleted from the buffer!
	 */
	void Read(TDes8& aDes);

	void AppendL(const TDesC8& aDes);
	
	inline TPtr8 Ptr() const;

	inline TInt Size() const;
	
	inline void Reset();
	
protected:

	CWriteBuffer();

	void ConstructL(TInt aGranularity = KDefaultWriteBrufferGranularity);

private:

//	CBufSeg* iBuffer;
	CBufFlat* iBuffer;

};

inline TInt CWriteBuffer::Size() const {
	return iBuffer->Size();
}

inline TPtr8 CWriteBuffer::Ptr() const {
	return iBuffer->Ptr(0);
}

inline void CWriteBuffer::Reset() {
	iBuffer->Reset();	
}

#endif
