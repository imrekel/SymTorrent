/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
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
 *  Name     : CSTPiece from STPiece.cpp
 *  Part of  : SymTorrent
 *  Created  : 10.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STPiece.h"
#include "STGlobal.h"
#include "SymTorrentEngineLog.h"
#include "STFile.h"
#include "STFileManager.h"
#include "hash.h"

// ================= MEMBER FUNCTIONS =======================

CSTPiece::CSTPiece(CSTTorrent& aTorrent, TBuf8<20>& aHash, TInt aTotalSize)
 : iTorrent(aTorrent), iHash(aHash), iTotalSize(aTotalSize)
{
}

CSTPiece::~CSTPiece()
{
	iFiles.Reset();
	iInsertedBlocks.Reset();
}

void CSTPiece::IncreaseDownloadedSizeL(TInt aSize, CSTPeer* aPeer)
{
	iDownloadedSize += aSize;
	iTorrent.UpdateBytesDownloadedL(aSize);
	
	if (Remaining() == 0)
	{
		LogPieceIndexL();
		LWRITELN(LOG, _L("Downloading piece completed!"));
		if (CheckHashL())
		{
			LogPieceIndexL();
			LWRITELN(LOG, _L("Hash OK!"));
			SetFilesDownloaded();
			iTorrent.PieceDownloadedL(this, aPeer);
		}
		else
		{
			LogPieceIndexL();
			LWRITELN(LOG, _L("Hash FAILED!"));
			iDownloadedSize = 0;
			iTorrent.PieceHashFailedL(this);
		}		
	}	
}

void CSTPiece::TryMergeInsertedBlocksL()
{
	TBool downloadedSizeChanged;
	
	do
	{
		downloadedSizeChanged = EFalse;
		
		for (TInt i=0; i<iInsertedBlocks.Count(); i++)
		{
			if (iInsertedBlocks[i].iBegin <= iDownloadedSize)
			{
				TInt mergedSize = (iInsertedBlocks[i].iBegin + iInsertedBlocks[i].iLength - iDownloadedSize);
				
				if (mergedSize > 0)
				{
					iDownloadedSize += mergedSize;
					iTorrent.UpdateBytesDownloadedL(mergedSize);
				}
				
				HLWRITE(LOG, _L("[CSTPiece] Block merged begin: "));
				HLWRITE(LOG, iInsertedBlocks[i].iBegin);
				HLWRITE(LOG, _L(" length: "));
				HLWRITE(LOG, iInsertedBlocks[i].iLength);
				HLWRITE(LOG, _L(" merged_size: "));
				HLWRITE(LOG, mergedSize);
				HLWRITE(LOG, _L(" new_downloaded_size: "));
				HLWRITELN(LOG, iDownloadedSize);
				
				downloadedSizeChanged = ETrue;
				
				iInsertedBlocks.Remove(i);
				break;
			}
		}
	}
	while (downloadedSizeChanged);
}

TInt CSTPiece::InsertBlockL(TInt aPosition, const TDesC8& aBlock, CSTPeer* aPeer)
{
	HLWRITE(LOG, _L("[CSTPiece] InsertBlockL, pos: "));
	HLWRITE(LOG, aPosition);
	HLWRITE(LOG, _L(" length: "));
	HLWRITE(LOG, aBlock.Length());
	HLWRITE(LOG, _L(" downloaded_size: "));
	HLWRITE(LOG, iDownloadedSize);
	HLWRITE(LOG, _L(" total_size: "));
	HLWRITELN(LOG, iTotalSize);
	
	if (aPosition + aBlock.Length() <= iDownloadedSize)
	{
		HLWRITELN(LOG, _L("[CSTPiece] Block already received"));
		return KErrNone;
	}
	
	// position inside the block (subpiece)
	TInt blockPosition = 0;
	
	TBool appendingBlock = (aPosition == iDownloadedSize); // if the inserted block is appended directly to the already downloaded part of the piece
	
	for (TInt i=0; i<iInsertedBlocks.Count(); i++)
	{
		if ((iInsertedBlocks[i].iBegin == aPosition) && (iInsertedBlocks[i].iLength == aBlock.Length()))
		{
			HLWRITELN(LOG, _L("[CSTPiece] Block already received"));
			return KErrNone;
		}
	}

	while (blockPosition < aBlock.Length())
	{
		CSTFile* file = NULL;
		TInt filePosition = 0;

		GetFilePosition(aPosition + blockPosition, &file, filePosition);

		if (file)
		{
			TInt res = KErrNone;
			// the remaining bytes from the file position until the end of the file
			TInt fileBlockLength = file->Size() - filePosition;

			LogPieceIndexL();
			LWRITE(LOG, _L("[CSTPiece] Appending blockpos "));
			LWRITE(LOG, blockPosition);
			LWRITE(LOG, _L(" to filepos "));
			LWRITE(LOG, filePosition);
			LWRITE(LOG, _L(" of "));
			LWRITELN(LOG, file->Path());

			if (fileBlockLength <= (aBlock.Length() - blockPosition))	// reached the end of a file, but the file is not neccessarily complete yet!
			{
				LogPieceIndexL();
				LWRITE(LOG, _L("[CSTPiece] Reached end of file: "));
				LWRITELN(LOG, file->Path());

				res = FileManager().WriteL(file, 
					filePosition, aBlock.Mid(blockPosition, fileBlockLength));
				if (res != KErrNone)
				{
					LWRITE(LOG, _L("[CSTPiece] File write failed: "));
					LWRITELN(LOG, res);
					
					return res;
				}

				FileManager().Close(file);

				blockPosition += fileBlockLength;
				
				if (appendingBlock)
					iDownloadedSize += fileBlockLength;				
			}
			else // the file isn't finished yet
			{
				res = FileManager().WriteL(file, 
					filePosition, aBlock.Right(aBlock.Length() - blockPosition));											

				if (res != KErrNone)
					return res;

				if (appendingBlock)
					iDownloadedSize += (aBlock.Length() - blockPosition);

				break;
			}
		}
		else
		{
			//User::Panic(KLitSymTorrent, ESTPanGetFilePositionFailed);
			
			LWRITELN(LOG, _L("File entry not found"));
			
			return KErrGeneral;
		}
	}

	if (!appendingBlock) // create a new inserted block entry
	{
		HLWRITELN(LOG, _L("[CSTPiece] Block inserted!"));
		iInsertedBlocks.AppendL(TSTInsertedBlock(aPosition, aBlock.Length()));
	}
	else
		iTorrent.UpdateBytesDownloadedL(aBlock.Length());
	
	if (appendingBlock || (aPosition < iDownloadedSize))
	{
		HLWRITELN(LOG, _L("[CSTPiece] Block appended, trying to merge!"));
		TryMergeInsertedBlocksL();
	}

	HLWRITE(LOG, _L("[Piece] Block insert succeeded, remaining bytes: "));
	HLWRITELN(LOG, Remaining());

	if (Remaining() == 0)
	{
		LogPieceIndexL();
		LWRITELN(LOG, _L("[CSTPiece] Downloading piece completed!"));
		if (CheckHashL())
		{
			LogPieceIndexL();
			LWRITELN(LOG, _L("[CSTPiece] Hash OK!"));
			SetFilesDownloaded();
			iTorrent.PieceDownloadedL(this, aPeer);
		}
		else
		{
			LogPieceIndexL();
			LWRITELN(LOG, _L("[CSTPiece] Hash FAILED!"));
			iDownloadedSize = 0;
			if (aPeer)
			{
				aPeer->FailedPieceCollector().RegisterFailedPieceL(*this);
			}
			iTorrent.PieceHashFailedL(this);
		}		
	}
	else
		if (Remaining() < 0)
		{
			LWRITELN(LOG, _L("[CSTPiece] ERROR, file remaining < 0"));
			return KErrGeneral;
		}

	return KErrNone;
}

TInt CSTPiece::AppendBlockL(const TDesC8& aBlock, CSTPeer* aPeer)
{
	return InsertBlockL(iDownloadedSize, aBlock, aPeer);
}

/*TInt CSTPiece::AppendBlockL(const TDesC8& aBlock, CSTPeer* aPeer)
{
	TInt blockPosition = 0;

	while (blockPosition < aBlock.Length())
	{
		CSTFile* file = NULL;
		TInt filePosition = 0;

		GetFilePosition(iDownloadedSize, &file, filePosition);

		if (file)
		{
			TInt res = KErrNone;
			TInt fileBlockLength = file->Size() - filePosition;

			LogPieceIndexL();
			LWRITE(LOG, _L("Appending block to "));
			LWRITELN(LOG, file->Path());

			if (fileBlockLength <= (aBlock.Length() - blockPosition))	// reached the end of a file
			{
				LogPieceIndexL();
				LWRITE(LOG, _L("Reached the end of file: "));
				LWRITELN(LOG, file->Path());

				res = FileManager().WriteL(file, 
					filePosition, aBlock.Mid(blockPosition, fileBlockLength));
				if (res != KErrNone)
					return res;

				FileManager().Close(file);

				blockPosition += fileBlockLength;
				iDownloadedSize += fileBlockLength;				
			}
			else // the file isn't finished yet
			{
				res = FileManager().WriteL(file, 
					filePosition, aBlock.Right(aBlock.Length() - blockPosition));											

				if (res != KErrNone)
					return res;

				iDownloadedSize += (aBlock.Length() - blockPosition);
				//blockPosition = aBlock.Length();
				break;
			}
		}
		else
			//User::Panic(KLitSymTorrent, ESTPanGetFilePositionFailed);
			return KErrGeneral;
	}

	iTorrent.UpdateBytesDownloadedL(aBlock.Length());

	//LOG->WriteL(_L("[Piece] Append succeeded, remaining bytes: "));
	//LOG->WriteLineL(Remaining());

	if (Remaining() == 0)
	{
		LogPieceIndexL();
		LWRITELN(LOG, _L("Downloading piece completed!"));
		if (CheckHashL())
		{
			LogPieceIndexL();
			LWRITELN(LOG, _L("Hash OK!"));
			SetFilesDownloaded();
			iTorrent.PieceDownloadedL(this, aPeer);
		}
		else
		{
			LogPieceIndexL();
			LWRITELN(LOG, _L("Hash FAILED!"));
			iDownloadedSize = 0;
			iTorrent.PieceHashFailedL(this);
		}		
	}
	else
		if (Remaining() < 0)
			return KErrGeneral;

	return KErrNone;
}*/

void CSTPiece::SetDownloadedSize(TInt aDownloadedSize)
{
	iDownloadedSize = aDownloadedSize;
	
	// TODO replace following code with assertation and panic
	if (iDownloadedSize >= iTotalSize)
		iDownloadedSize = 0;
}

void CSTPiece::SetDownloaded(TBool aDownloaded)
{
	if (aDownloaded)
	{
		iDownloadedSize = iTotalSize;
		SetFilesDownloaded();
	}
	else
	{
		iDownloadedSize = 0;
	
		for (TInt i=0; i<iFiles.Count(); i++)
			iFiles[i].iFile.SetDownloaded(EFalse);
	}
}

void CSTPiece::SetFilesDownloaded() // !!! SHOULD BE REVISED !!!
{
	for (TInt i=0; i<iFiles.Count(); i++)
	{
		TBool downloaded = ETrue;
		for (TInt j=0; j<iTorrent.PieceCount(); j++)
		{
			if ((iTorrent.Piece(j)->HasFile(&iFiles[i].iFile)) &&
				!(iTorrent.Piece(j)->IsDownloaded()))
			{
				downloaded = EFalse;
				break;
			}						
		}
		
		if (downloaded)
		{	
			LWRITE(LOG, _L("[File] "));
			LWRITE(LOG, iFiles[i].iFile.Path());
			LWRITELN(LOG, _L(" downloaded"));
			iFiles[i].iFile.SetDownloaded();
			FileManager().Close(&(iFiles[i].iFile));
		}			
	}
}


void CSTPiece::GetFilePosition(TInt aPositionInPiece, CSTFile** aFile, TInt& aPositionInFile)
{
	TInt pos = 0;

	for (TInt i=0; i<iFiles.Count(); i++)
	{
		if (aPositionInPiece < (pos + iFiles[i].iLength))
		{
			*aFile = &(iFiles[i].iFile);
			aPositionInFile = iFiles[i].iOffset + (aPositionInPiece - pos);
			return;
		}
		else
			pos += iFiles[i].iLength;
	}

	*aFile = NULL;
	return;
}


HBufC8* CSTPiece::GetBlockL(TInt aPosition, TInt aLength)
{	
	TInt filePos = 0;
	
	HBufC8* block = HBufC8::NewLC(aLength);
	TPtr8 blockPtr(block->Des());
	
	CSTFile* file = NULL;

	TInt i=0;
	TInt pos = 0;
	for (; i<iFiles.Count(); i++)
	{
		if (aPosition < (pos + iFiles[i].iLength))
		{
			file = &(iFiles[i].iFile);
			filePos = iFiles[i].iOffset + (aPosition - pos);
			break;
		}
		else
			pos += iFiles[i].iLength;
	}
	
	if (file == NULL)
	{
		CleanupStack::PopAndDestroy(); // block
		return NULL;		
	}
	
	while (aLength > 0)
	{
		if (TUint(aLength) >= (file->Size() - filePos))
		{
			HBufC8* buf = FileManager().ReadL(file, filePos, file->Size() - filePos);
			if (buf)
			{
				blockPtr.Append(*buf);
				delete buf;
				aLength -= (file->Size() - filePos);
			}
			else
				break;
		}
		else
		{
			HBufC8* buf = FileManager().ReadL(file, filePos, aLength);
			if (buf)
			{
				blockPtr.Append(*buf);
				aLength = 0;
				delete buf;				
			}
			
			break;		
		}
		
		if (i >= iFiles.Count()-1)
			break;
		
		file = &iFiles[++i].iFile;
		filePos = 0;
	}
	
	if (aLength != 0)
	{
		CleanupStack::PopAndDestroy(); // block
		return NULL;		
	}
		
	CleanupStack::Pop(); // block
	return block;	
}


void CSTPiece::AddFileFragmentL(CSTFile& aFile, TUint aOffset, TUint aLength)
{
	//LOG->WriteL(_L("[Piece "));
	//LOG->WriteL(Index());
	//LOG->WriteL(_L("] Adding file fragment of "));
	//LOG->WriteL(aFile.Path());
	//LOG->WriteL(_L(" Offset: "));
	//LOG->WriteL(aOffset);
	//LOG->WriteL(_L(" Length: "));
	//LOG->WriteLineL(aLength);

	User::LeaveIfError(iFiles.Append(TSTFileFragment(aFile, aOffset, aLength)));
}


TBool CSTPiece::CheckHashL()
{
//	if (Remaining() != 0)
//		return EFalse;
	
	TBool result = EFalse;
	
	HBufC8* piece = GetBlockL(0, TotalSize());
	if (piece)
	{
		CleanupStack::PushL(piece);
		CSHA1* sha1 = CSHA1::NewL();
		CleanupStack::PushL(sha1);
		if (sha1->Hash(*piece) == iHash)
			result = ETrue;
		
	//	LOG->WriteL(_L("[Piece] Hash from .torrent file   : "));
	//	LOG->WriteLineL(iHash);
		
	//	LOG->WriteL(_L("[Piece] Hash of the received piece: "));
	//	LOG->WriteLineL(sha1->Hash(*piece));

		CleanupStack::PopAndDestroy(); // sha1
		CleanupStack::PopAndDestroy(); // piece
	}
	
	return result;
}

TBool CSTPiece::HasFile(CSTFile* aFile)
{
	for (TInt i=0; i<iFiles.Count(); i++)
		if (&iFiles[i].iFile == aFile)
			return ETrue;
		
	return EFalse;
}

void CSTPiece::LogPieceIndexL()
{
	#ifdef LOG_TO_FILE
		TBuf<64> buf;
		buf.Format(_L("[Piece %d] "), Index());
		LOG->WriteL(buf);
	#endif
}

TBool CSTPiece::IsAllFilesSkipped() const 
{
	for (TInt i=0; i<iFiles.Count(); i++)
	{
		if (!iFiles[i].iFile.IsSkipped())
			return EFalse;
	}
	
	return ETrue;
}

void CSTPiece::GetNextBlock(TInt aBegin, TInt aMaxBlockSize, TInt& aBlockSize, TInt& aNewRequestedSize)
{
	aBlockSize = (iTotalSize - aBegin);
	if (aBlockSize > aMaxBlockSize)
		aBlockSize = aMaxBlockSize;
	
	aNewRequestedSize = aBegin + aBlockSize;
			
	if (iInsertedBlocks.Count() > 0)
	{
		TInt smallestPosition = iInsertedBlocks[0].iBegin;
		
		for (TInt i=0; i<iInsertedBlocks.Count(); i++)
		{
			if ((iInsertedBlocks[i].iBegin < smallestPosition) && (iInsertedBlocks[i].iBegin > aBegin))
				smallestPosition = iInsertedBlocks[i].iBegin;
		}
		
		if (smallestPosition > aBegin)
		{
			aBlockSize = smallestPosition - aBegin;
			if (aBlockSize > aMaxBlockSize)
				aBlockSize = aMaxBlockSize;
			
			aNewRequestedSize = aBegin + aBlockSize;
			
			TBool requestedSizeChanged = EFalse;
			
			do
			{
				requestedSizeChanged = EFalse;
				
				for (TInt i=0; i<iInsertedBlocks.Count(); i++)
				{
					if ((iInsertedBlocks[i].iBegin <= aNewRequestedSize) && (iInsertedBlocks[i].iBegin + iInsertedBlocks[i].iLength > aNewRequestedSize))
					{
						aNewRequestedSize = iInsertedBlocks[i].iBegin + iInsertedBlocks[i].iLength;
						requestedSizeChanged = ETrue;
						break;
					}
				}
			}
			while(requestedSizeChanged);
		}
		
	}
}

/*TInt CSTPiece::RemainingBlockSizeUntilNextInsertedSubPiece(TInt aBegin) const
{
	if (iInsertedBlocks.Count() > 0)
	{
		TInt smallestPosition = iInsertedBlocks[0].iBegin;
		
		for (TInt i=0; i<iInsertedBlocks.Count(); i++)
		{
			if ((iInsertedBlocks[i].iBegin < smallestPosition) && (iInsertedBlocks[i].iBegin > aBegin))
				smallestPosition = iInsertedBlocks[i].iBegin;
		}
		
		if (smallestPosition > aBegin)
			return (smallestPosition - aBegin);
		
	}

	return (iTotalSize - aBegin);
}*/


