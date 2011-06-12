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
 *  Name     : CSTFileManager from STFileManager.cpp
 *  Part of  : SymTorrent
 *  Created  : 03.03.2006 by Imre Kelényi
 * ============================================================================
 */

#include "STFileManager.h"
#include "STFileManagerEntry.h"

const TInt KMaxFileManagerEntries = 20;

// ================= MEMBER FUNCTIONS =======================

CSTFileManager::CSTFileManager(CSTTorrent& aTorrent, RFs& aFs)
 : iTorrent(aTorrent), iFs(aFs)
{
	// No implementation required
}

CSTFileManager::~CSTFileManager()
{
	iFiles.ResetAndDestroy();
}

CSTFileManager* CSTFileManager::NewLC(CSTTorrent& aTorrent, RFs& aFs)
{
	CSTFileManager* self = new (ELeave) CSTFileManager(aTorrent, aFs);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CSTFileManager* CSTFileManager::NewL(CSTTorrent& aTorrent, RFs& aFs)
{
	CSTFileManager* self=CSTFileManager::NewLC(aTorrent, aFs);
	CleanupStack::Pop(); // self;
	return self;
}

void CSTFileManager::ConstructL()
{
}

CSTFileManagerEntry* CSTFileManager::GetFileManagerEntryL(CSTFile* aFile)
{
	CSTFileManagerEntry* fileEntry = NULL;
	TInt fileEntryIndex = -1;

	for (TInt i=0; i<iFiles.Count(); i++)
	{
		if (&(iFiles[i]->FileId()) == aFile)
		{
			fileEntryIndex = i;
			break;
		}
	}
	
	if (fileEntryIndex < 0)
	{
		fileEntry = AddFileL(aFile);
	}
	else
		fileEntry = iFiles[fileEntryIndex];
	
	return fileEntry;
}

TInt CSTFileManager::WriteL(CSTFile* aFile, TInt aPosition, const TDesC8& aData)
{
	CSTFileManagerEntry* fileEntry = GetFileManagerEntryL(aFile);
	if (fileEntry)
		return fileEntry->Write(aPosition, aData);
	else
		return KErrGeneral;
}

void CSTFileManager::WriteAsyncL(CSTFile* aFile, TInt aPosition, const TDesC8& aData, MSTFileWriteObserver* aFileWriteObserver)
{
	CSTFileManagerEntry* fileEntry = GetFileManagerEntryL(aFile);
	User::LeaveIfNull(fileEntry);
	
	fileEntry->WriteAsyncL(aPosition, aData, aFileWriteObserver);
}

void CSTFileManager::Close(CSTFile* aFile)
{
	TInt fileEntryIndex = -1;

	for (TInt i=0; i<iFiles.Count(); i++)
	{
		if (&(iFiles[i]->FileId()) == aFile)
		{
			fileEntryIndex = i;
			break;
		}
	}
	
	if (fileEntryIndex >= 0)
	{
		delete iFiles[fileEntryIndex];
		iFiles.Remove(fileEntryIndex);
	}
}


void CSTFileManager::CloseAll()
{
	iFiles.ResetAndDestroy();
}


HBufC8* CSTFileManager::ReadL(CSTFile* aFile, TInt aPos, TInt aLength)
{
	//HLWRITELN(iLog, _L("[FileManager] ReadL begin"));
	
	CSTFileManagerEntry* fileEntry = GetFileManagerEntryL(aFile);
	if (fileEntry)
		return fileEntry->ReadL(aPos, aLength);
	else
		return NULL;
	
	//HLWRITELN(iLog, _L("[FileManager] ReadL end"));
}


CSTFileManagerEntry* CSTFileManager::AddFileL(CSTFile* aFile)
{
	if (iFiles.Count() >= KMaxFileManagerEntries)
	{
		for (TInt i=iFiles.Count() - 1; i >= 0; i--)
			if (!iFiles[i]->HasPendingFileOperations())
			{
				delete iFiles[i];
				iFiles.Remove(i);
				
				if (iFiles.Count() < KMaxFileManagerEntries)
					break;
			}		
	}

	CSTFileManagerEntry* file = CSTFileManagerEntry::NewLC(iFs, iTorrent, *aFile); // TODO TRAPD?
	//if (err)
	//	return NULL;

	User::LeaveIfError(iFiles.Insert(file, 0));
	CleanupStack::Pop(); // file

	return file;
}
