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

/*
 * ============================================================================
 *  Classes  : CSTFile
 *			  
 *  Part of  : SymTorrent
 *  Created  : 10.02.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STFILE_H
#define SYMTORRENT_STFILE_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include "STDefs.h"

// FORWARD DECLARATIONS
class CSTTorrent;
class CSTFile;

/**
 * CSTFile
 *
 * A piece of a torrent.
 */
class CSTFile : public CBase
{
public:

	IMPORT_C static CSTFile* NewL(const TDesC& aPath, TInt64 aSize);
	
	IMPORT_C static CSTFile* NewLC(const TDesC& aPath, TInt64 aSize);	
	
	IMPORT_C ~CSTFile();
	
	inline TInt64 Size() const;
	
	/**
	 * @return the full path to the file (directories and filename)
	 */
	inline const TDesC& Path() const;
	
	/**
	 * @return the name of the file (without directory path)
	 */
	inline TPtrC FileName() const;
	
	inline TBool IsDownloaded() const;
	
	inline void SetDownloaded(TBool aDownloaded = ETrue);
	
	inline void SetSkipped(TBool aSkipped = ETrue);
	
	inline TBool IsSkipped() const;
	
	/**
	 * Creates a new HBufC containing information about the file. 
	 * The caller takes the ownership of the returned buffer.
	 */
	IMPORT_C HBufC* CreateFileInfoL() const;

private: // constructors

	CSTFile(TInt64 aSize);

	void ConstructL(const TDesC& aPath);
	
private:

	RFile iFile;

	TInt64 iSize;

	/**
	 * Path relative to the torrent's parent directory.
	 */
	HBufC* iPath;
	
	TBool iDownloaded;
	
	TBool iSkipped;
	
	TInt iFileNameLength;
};

// INLINE FUNCTION IMPLEMENTATIONS

inline TInt64 CSTFile::Size() const {
	return iSize;	
}

inline const TDesC& CSTFile::Path() const {
	return *iPath;
}

inline TBool CSTFile::IsDownloaded() const {
	return iDownloaded;
}
	
inline void CSTFile::SetDownloaded(TBool aDownloaded) {
	iDownloaded = aDownloaded;
}

inline TPtrC CSTFile::FileName() const {
	return iPath->Right(iFileNameLength);
}

inline void CSTFile::SetSkipped(TBool aSkipped) {
	iSkipped = aSkipped;
}

inline TBool CSTFile::IsSkipped() const {
	return iSkipped;
}

#endif
