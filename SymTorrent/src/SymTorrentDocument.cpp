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
*  Name     : CSymTorrentDocument from SymTorrentDocument.h
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#include "SymTorrentDocument.h"
#include "SymTorrentAppui.h"
#include "STTorrentManagerSingleton.h"
#include "SymTorrentLog.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
CSymTorrentDocument::CSymTorrentDocument(CEikApplication& aApp)
 : CAknDocument(aApp)    
    {
    }

// destructor
CSymTorrentDocument::~CSymTorrentDocument()
{
#ifdef LOG_TO_FILE
	CKiLogManager::Free();
#endif
}

// EPOC default constructor can leave.
void CSymTorrentDocument::ConstructL()
{
#ifdef LOG_TO_FILE
	CKiLogManager::InitializeL();
#endif
}

// Two-phased constructor.
CSymTorrentDocument* CSymTorrentDocument::NewL(
        CEikApplication& aApp)     // CSymTorrentApp reference
    {
    CSymTorrentDocument* self = new (ELeave) CSymTorrentDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }
    
// ----------------------------------------------------
// CSymTorrentDocument::CreateAppUiL()
// constructs CSymTorrentAppUi
// ----------------------------------------------------
//
CEikAppUi* CSymTorrentDocument::CreateAppUiL()
    {
    return new (ELeave) CSymTorrentAppUi;
    }


#ifdef EKA2
void CSymTorrentDocument::OpenFileL(CFileStore*& /*aFileStore*/, RFile& aFile)
	{
	TORRENTMGR->OpenTorrentL(aFile);
	}
#else
CFileStore* CSymTorrentDocument::OpenFileL(TBool /*aDoOpen*/, const TDesC& aFilename, RFs& /*aFs*/)
	{
	LOG->WriteL(_L("OpenFileL: "));
	LOG->WriteLineL(aFilename);
	TORRENTMGR->OpenTorrentDelayedL(aFilename);
	return NULL;
	}
#endif



// End of File  
