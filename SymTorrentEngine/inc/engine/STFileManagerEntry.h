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

/**
 * ============================================================================
 *  Classes  : CSTFileManagerEntry
 *			  
 *  Part of  : SymTorrent engine
 *  Created  : 03.03.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STFILEMANAGERENTRY_H
#define SYMTORRENT_STFILEMANAGERENTRY_H

// INCLUDES
#include <f32file.h>
#include "STDefs.h"

// CLASS DECLARATION
class CSTFile;
class CSTTorrent;
class CSTFileWriter;
class MSTFileWriteObserver;

/**
 *  CSTFileManagerEntry
 */
class CSTFileManagerEntry : public CBase
{
public: // Constructors and destructor

	/**
     * Destructor.
     */
	~CSTFileManagerEntry();

    /**
     * Two-phased constructor. Leaves if it fails to open/replace file.
     */
	static CSTFileManagerEntry* NewL(RFs& aFs, CSTTorrent& aTorrent, CSTFile& aFile);

    /**
     * Two-phased constructor. Leaves if it fails to open/replace file.
     */
	static CSTFileManagerEntry* NewLC(RFs& aFs, CSTTorrent& aTorrent, CSTFile& aFile);

	inline CSTFile& FileId() const;

	TInt Write(TInt aPosition, const TDesC8& aData);
	
	void WriteAsyncL(TInt aPosition, const TDesC8& aData, MSTFileWriteObserver* aFileWriteObserver);
	
	HBufC8* ReadL(TInt aPos, TInt aLength);
	
	/**
	 * Called when an asynchronous file write operation completes
	 */
	void OnFileWriteCompleteL(CSTFileWriter* aFileWriter);

	/**
	 * @return ETrue if there is any active write or read requests
	 */
	TBool HasPendingFileOperations() const;
	
	inline RFile& FileHandle();

	inline CSTFile& File();


private:

	/**
     * Constructor for performing 1st stage construction
     */
	CSTFileManagerEntry(CSTFile& aFile);

	/**
     * EPOC default constructor for performing 2nd stage construction
     */
	void ConstructL(RFs& aFs, CSTTorrent& aTorrent);

private:

	RFile iFileHandle;

	CSTFile& iFile;
	
	RPointerArray<CSTFileWriter> iFileWriters;

	//TInt iReferenceCounter;
};

// INLINE FUNCTION IMPLEMENTATIONS

inline CSTFile& CSTFileManagerEntry::FileId() const {
	return iFile;
}

inline RFile& CSTFileManagerEntry::FileHandle() {
	return iFileHandle;
}

inline CSTFile& CSTFileManagerEntry::File() {
	return iFile;
}


#endif // STFILEMANAGERENTRY_H
