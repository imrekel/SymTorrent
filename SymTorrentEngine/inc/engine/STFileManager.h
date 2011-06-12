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
 *  Classes  : CSTFileManager 
 *			  
 *  Part of  : SymTorrent engine
 *  Created  : 03.03.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STFILEMANAGER_H
#define SYMTORRENT_STFILEMANAGER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include "STDefs.h"

// CLASS DECLARATION
class CSTFileManagerEntry;
class CSTTorrent;
class CSTFile;

class MSTFileWriteObserver
{
public:
	virtual void OnFileWriteCompleteL(CSTFile& aFile, TInt aPosition) = 0;
	virtual void OnFileWriteFailedL(CSTFile& aFile, TInt aPosition) = 0;
};

/**
 *  CSTFileManager
 */
class CSTFileManager : public CBase
{
public: // Constructors and destructor

	/**
     * Destructor.
     */
	~CSTFileManager();

    /**
     * Two-phased constructor.
     */
	static CSTFileManager* NewL(CSTTorrent& aTorrent, RFs& aFs);

    /**
     * Two-phased constructor.
     */
	static CSTFileManager* NewLC(CSTTorrent& aTorrent, RFs& aFs);
	
	/**
	 * Writes a block of data to the specified file position.
	 * This is a synchronous operation.
	 */
	TInt WriteL(CSTFile* aFile, TInt aPosition, const TDesC8& aData);
	
	/**
	 * Writes a block of data to the specified file position asynchronously.
	 */
	void WriteAsyncL(CSTFile* aFile, TInt aPosition, const TDesC8& aData, MSTFileWriteObserver* aFileWriteObserver);

	//TInt ReadL(CSTFile* aFile, TInt aPosition, TInt aLength, TDes8& aBuffer);

	HBufC8* ReadL(CSTFile* aFile, TInt aPos, TInt aLength);

	/**
	 * Closes the specified file (if the file is not opened than does nothing)
	 */
	void Close(CSTFile* aFile);

	/**
	 * Closes all opened files
	 */
	void CloseAll();

private:

	/**
     * Constructor for performing 1st stage construction
     */
	CSTFileManager(CSTTorrent& aTorrent, RFs& aFs);

	/**
     * EPOC default constructor for performing 2nd stage construction
     */
	void ConstructL();

private:

	/**
	 * Opens or creates a file and adds it to the opened files pool
	 */
	CSTFileManagerEntry* AddFileL(CSTFile* aFile);
	
	CSTFileManagerEntry* GetFileManagerEntryL(CSTFile* aFile);

private:
	
	CSTTorrent& iTorrent;

	RFs& iFs;

	RPointerArray<CSTFileManagerEntry> iFiles;

};

#endif // STFILEMANAGER_H
