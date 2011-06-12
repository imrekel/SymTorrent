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
 *  Name     : CSTFileManagerEntry from STFileManagerEntry.cpp
 *  Part of  : SymTorrent
 *  Created  : 03.03.2006 by Imre Kelényi
 * ============================================================================
 */

#include "STFileManagerEntry.h"
#include "STFile.h"
#include "STTorrent.h"
#include "SymTorrentEngineLog.h"
#include "STFileWriter.h"

_LIT(KLitOpeningFileFailed, "Open/replace failed");

// ================= MEMBER FUNCTIONS =======================

CSTFileManagerEntry::CSTFileManagerEntry(CSTFile& aFile)
 : iFile(aFile)
{
	// No implementation required
}


CSTFileManagerEntry::~CSTFileManagerEntry()
{
	iFileWriters.ResetAndDestroy();
	iFileHandle.Close();
}

CSTFileManagerEntry* CSTFileManagerEntry::NewLC(RFs& aFs, CSTTorrent& aTorrent, CSTFile& aFile)
{
	CSTFileManagerEntry* self = new (ELeave)CSTFileManagerEntry(aFile);
	CleanupStack::PushL(self);
	self->ConstructL(aFs, aTorrent);
	return self;
}

CSTFileManagerEntry* CSTFileManagerEntry::NewL(RFs& aFs, CSTTorrent& aTorrent, CSTFile& aFile)
{
	CSTFileManagerEntry* self = CSTFileManagerEntry::NewLC(aFs, aTorrent, aFile);
	CleanupStack::Pop(); // self;
	return self;
}

void CSTFileManagerEntry::ConstructL(RFs& aFs, CSTTorrent& /*aTorrent*/)
{
	/*HBufC* fileName = HBufC::NewLC(aTorrent.Path().Length() + iFile.Path().Length());
	TPtr fileNamePtr(fileName->Des());
	fileNamePtr.Copy(aTorrent.Path());
	fileNamePtr.Append(iFile.Path());*/		

	TPtrC fileNamePtr = iFile.Path();
	aFs.MkDirAll(iFile.Path());
	
	#ifdef LOG_TO_FILE
		CKiLogger* log = STLOG;
		log->WriteL(_L("[File] Opening "));
		log->WriteLineL(fileNamePtr);
	#endif

	if (iFileHandle.Open(aFs, fileNamePtr, EFileWrite|EFileShareAny) != KErrNone)
	{
		LWRITELN(log, _L("[File] Cannot open file, trying to create/replace"));
		if (iFileHandle.Replace(aFs, fileNamePtr, EFileWrite|EFileShareAny) != KErrNone)
		{
			LWRITELN(log, _L("[File] Create/replace failed"));
			User::Leave(0);	// TODO replace
		}
		else
			LWRITELN(log, _L("[File] File created"));
	}
	else
		LWRITELN(log, _L("[File] File opened"));
}

TInt CSTFileManagerEntry::Write(TInt aPosition, const TDesC8& aData)
{
	HLWRITELN(STLOG, _L("[FileManagerEntry] Write begin"));
	
	TInt fileSize;
	TInt res;
	if ((res = iFileHandle.Size(fileSize)) != KErrNone)
		return res;

	if (fileSize < aPosition)
	{
		if ((res = iFileHandle.SetSize(aPosition)) != KErrNone)
		{
			LWRITELN(STLOG, _L("[File] SetSize error"));
			return res;
		}				
	}

	return iFileHandle.Write(aPosition, aData);
}


HBufC8* CSTFileManagerEntry::ReadL(TInt aPos, TInt aLength)
{
	HLWRITELN(STLOG, _L("[FileManagerEntry] ReadL begin"));
	
	HBufC8* buf = HBufC8::NewLC(aLength);
	TPtr8 bufPtr(buf->Des());
	
	TInt res;
	
	res = iFileHandle.Read(aPos, bufPtr, aLength);
	
	if (res == KErrNone)
	{
		CleanupStack::Pop(); // buf
		
		HLWRITELN(STLOG, _L("[FileManagerEntry] ReadL end"));
		
		return buf;
	}
	else
	{
		CleanupStack::PopAndDestroy(); // buf
		
		HLWRITELN(STLOG, _L("[FileManagerEntry] ReadL end (error!)"));
		
		return NULL;
		
	}
}

void CSTFileManagerEntry::OnFileWriteCompleteL(CSTFileWriter* aFileWriter)
{
	TInt index = iFileWriters.Find(aFileWriter);
	
	if (index >= 0)
	{
		iFileWriters.Remove(index);
		delete aFileWriter;
	}
}

void CSTFileManagerEntry::WriteAsyncL(TInt aPosition, const TDesC8& aData, MSTFileWriteObserver* aFileWriteObserver)
{
	CSTFileWriter* fileWriter = CSTFileWriter::NewLC();
	iFileWriters.AppendL(fileWriter);
	CleanupStack::Pop(); // fileWriter
	
	fileWriter->StartWrite(this, aPosition, aData, aFileWriteObserver);
}

TBool CSTFileManagerEntry::HasPendingFileOperations() const
{
	return (iFileWriters.Count() > 0);
}
