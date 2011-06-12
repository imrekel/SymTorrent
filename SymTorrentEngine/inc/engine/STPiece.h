/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrentEngine
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
 *  Classes  : CSTPiece
 *             TSTFileFragment
 *			  
 *  Part of  : SymTorrent
 *  Created  : 10.02.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STPIECE_H
#define SYMTORRENT_STPIECE_H

// INCLUDES
#include <e32base.h>
#include "STTorrent.h"
#include "STDefs.h"

// FORWARD DECLARATIONS
class CSTTorrent;
class CSTFile;
class CSTPeer;

/**
 * TSTFileFragment
 */
class TSTFileFragment
{
public:

	TSTFileFragment(CSTFile& aFile, TUint aOffset, TUint aLength)
	 : iFile(aFile), iOffset(aOffset), iLength(aLength)
	{	 	
	}	
	
public:

	/**
	 * The file which contains the fragment
	 */
	CSTFile& iFile;

	/**
	 * The starting position of the fragment in the file
	 */
	TInt iOffset;
	
	/**
	 * The length of the fragment in bytes
	 */
	TInt iLength;
};

/**
 * A block that was not appended directly to the downloaded part of the piece but was inserted. 
 * 
 * Inserted blocks are merged with the downloaded part
 * of the piece when the end of the downloaded part from the beginning of the pieces reaces the beginning of
 * an inserted block.
 */
class TSTInsertedBlock
{
public:
	
	TSTInsertedBlock(TInt aBegin, TInt aLength)
	 : iBegin(aBegin), iLength(aLength) {}
	
public:
	TInt iBegin;
	TInt iLength;
};


/**
 * CSTPiece
 *
 * A piece of a torrent.
 */
class CSTPiece : public CBase
{
public:

	CSTPiece(CSTTorrent& aTorrent, TBuf8<20>& aHash, TInt aTotalSize);

	~CSTPiece();

	/**
	 * Writes a block of downloaded bytes to the piece
	 */
	TInt AppendBlockL(const TDesC8& aBlock, CSTPeer* aPeer);
	
	TInt InsertBlockL(TInt aPosition, const TDesC8& aBlock, CSTPeer* aPeer);
	
	/**
	 * @return the size of the donwloaded part
	 */
	inline TInt DownloadedSize() const;
	
	/**
	 * @return the total length of the piece
	 */
	inline TInt TotalSize() const;

	/**
	 * @return the size of the remaining part
	 */
	inline TInt Remaining() const;
	
	/**
	 * @return the index of the piece
	 */
	inline TInt Index() const;
	
	/**
	 * Extracts a block of data from the piece and returns it in a newly created HBufC8 (the caller
	 * takes ownership).
	 */
	HBufC8* GetBlockL(TInt aPosition, TInt aLength);
	
	/**
	 * Calculates the hash value of the download piece and compares it with the hash from
	 * the torrent file.
	 *
	 * @return true if the hash matches, false if doesn't
	 */
	TBool CheckHashL();

	void AddFileFragmentL(CSTFile& aFile, TUint aOffset, TUint aLength);
	
	inline TInt NumberOfPeersHaveThis() const;
	
	inline void IncNumberOfPeersHaveThis();
	
	inline void DecNumberOfPeersHaveThis();
	
	inline TBool IsDownloaded() const;
	
	TBool HasFile(CSTFile* aFile);
	
	TBool IsAllFilesSkipped() const;
	
	/**
	 * Resets the downloaded size. Can only be used if the piece is not complete!
	 */
	void SetDownloadedSize(TInt aDownloadedSize);
	
	/**
	 * Sets the piece's download state
	 *
	 * @param aDownloaded ETrue to set the piece downloaded, EFalse if it's not downloaded
	 */
	void SetDownloaded(TBool aDownloaded = ETrue);
	
	/**
	 * Returns the size of the next missing block beginning at aBegin, plus the size of the already requested
	 * part of the piece
	 */
	void GetNextBlock(TInt aBegin, TInt aMaxBlockSize, TInt& aBlockSize, TInt& aNewRequestedSize);	

private:

	/**
	 * Calculates file position from a piece position.
	 */
	void GetFilePosition(TInt aPositionInPiece, CSTFile** aFile, TInt& aPositionInFile);

	inline CSTFileManager& FileManager();	
	
	void SetFilesDownloaded(); // !!! SHOULD BE REVISED !!!
	
	void LogPieceIndexL();
	
	/**
	 * Update the downloaded bytes.
	 * 
	 * @aPeer the peer from which the block of the piece is received
	 */
	void IncreaseDownloadedSizeL(TInt aSize, CSTPeer* aPeer);
	
	/**
	 * Tries to merge the inserted blocks with the "downloaded part" of the piece
	 */
	void TryMergeInsertedBlocksL();

private: // data

	/**
	 * Reference to the torrent the piece belongs to.
	 */
	CSTTorrent&	iTorrent;
	
	/**
	 * The 20 byte long SHA1 value of the piece (came from the torrent file).
	 */
	TBuf8<20> iHash;

	TInt iNumberOfPeersHaveThis;

	/**
	 * The number of bytes already downloaded.
	 */
	TInt iDownloadedSize;

	TInt iTotalSize;	
	
	RArray<TSTFileFragment> iFiles;
	
	RArray<TSTInsertedBlock> iInsertedBlocks;
	
	friend class CSTPieceAccess;
};

// INLINE FUNCTION IMPLEMENTATIONS

inline TInt CSTPiece::DownloadedSize() const {
	return iDownloadedSize;
}

inline TInt CSTPiece::TotalSize() const {
	return iTotalSize;
}

inline TInt CSTPiece::Index() const {
	return iTorrent.IndexOfPiece(this);
}

inline CSTFileManager& CSTPiece::FileManager() {
	return iTorrent.FileManager();
}

inline TInt CSTPiece::Remaining() const {
	return TotalSize() - iDownloadedSize;
}

inline TInt CSTPiece::NumberOfPeersHaveThis() const {
	return iNumberOfPeersHaveThis;
	
}

inline void CSTPiece::IncNumberOfPeersHaveThis() {
	iNumberOfPeersHaveThis++;	
}
	
inline void CSTPiece::DecNumberOfPeersHaveThis() {
	iNumberOfPeersHaveThis--;
}

inline TBool CSTPiece::IsDownloaded() const {
	return (TotalSize() == DownloadedSize());
}


#endif
