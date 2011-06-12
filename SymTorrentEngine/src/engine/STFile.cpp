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
 *  Name     : CSTFile from STFile.cpp
 *  Part of  : SymTorrent
 *  Created  : 10.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STFile.h"
#include "STStringFormatter.h"

// ================= MEMBER FUNCTIONS =======================

EXPORT_C CSTFile* CSTFile::NewL(const TDesC& aPath, TInt64 aSize)
{
	CSTFile* self = CSTFile::NewLC(aPath, aSize);
	CleanupStack::Pop();
	
	return self;	
}
	
	
EXPORT_C CSTFile* CSTFile::NewLC(const TDesC& aPath, TInt64 aSize)
{
	CSTFile* self = new (ELeave) CSTFile(aSize);
	CleanupStack::PushL(self);
	self->ConstructL(aPath);
	
	return self;	
}


CSTFile::CSTFile(TInt64 aSize)
 : iSize (aSize)
{
}


void CSTFile::ConstructL(const TDesC& aPath)
{
	iPath = aPath.AllocL();
	TPtr path = iPath->Des();
	
	if (path.Length() > 0)	
		for (iFileNameLength = 1; iFileNameLength <= path.Length(); iFileNameLength++)
		{
			if (path[path.Length() - iFileNameLength] == '\\')
			{
				iFileNameLength--;
				break;
			}				
		}
}

	
EXPORT_C CSTFile::~CSTFile()
{
	delete iPath;
}


EXPORT_C HBufC* CSTFile::CreateFileInfoL() const
{
	HBufC* fileInfo = HBufC::NewLC(iPath->Length() + 100);
	TPtr fiPtr(fileInfo->Des());
	
	fiPtr.Insert(0, *iPath);
	fiPtr.Append(_L("\n"));
		
	TSTStringFormatter::AppendFileLength(iSize, fiPtr);
	fiPtr.Append(_L("\n"));
	
	if (iDownloaded)
		fiPtr.Append(_L("complete"));
	else
		fiPtr.Append(_L("not complete"));
	
	CleanupStack::Pop(); // fileInfo
	
	return fileInfo;
}
