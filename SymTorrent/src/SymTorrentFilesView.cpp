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
*  Name     : CSymTorrentFilesView from SymTorrentFilesView.h
*  Part of  : SymTorrent
*  Created  : 27.04.2006 by Péter Ekler
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <SymTorrent.rsg>
#include <caknfileselectiondialog.h>
#include <caknmemoryselectiondialog.h> 
#include <aknnotewrappers.h>
#include <aknmessagequerydialog.h>
//#include <apmstd.h>
#include "SymTorrentFilesView.h"
#include "SymTorrentFilesContainer.h" 
#include "Symtorrent.hrh"
#include "STGlobal.h"

#include "STTorrentManager.h"
#include "STTorrent.h"
#include "STFile.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentFilesView::ConstructL(CSTTorrentManager* aTorrentMgr, 
//   CSymTorrentAppUi* aAppUi)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentFilesView::ConstructL(CSTTorrentManager* aTorrentMgr, 
	CSymTorrentAppUi* aAppUi)
{
	iAppUi=aAppUi;
	iTorrentMgr=aTorrentMgr;	
    BaseConstructL( R_SYMTORRENT_FILESVIEW );
}

// ---------------------------------------------------------
// CSymTorrentFilesView::~CSymTorrentFilesView()
// destructor
// ---------------------------------------------------------
//
CSymTorrentFilesView::~CSymTorrentFilesView()
{
    if ( iFilesContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iFilesContainer );
    }
    
    delete iFilesContainer;
    iFilesContainer = NULL;
}

// ---------------------------------------------------------
// TUid CSymTorrentFilesView::Id()
//
// ---------------------------------------------------------
//
TUid CSymTorrentFilesView::Id() const
{
    return TUid::Uid(ESymTorrentFilesView);
}

// ---------------------------------------------------------
// CSymTorrentFilesView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentFilesView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
        {
        case ESymTorrentCmdDetailsFromTorrentDetails:
            {
            iAppUi->ActivateTorrentDetailViewL();
            break;
            }
            
        case ESymTorrentCmdDownloadstate:
            {
            iAppUi->ActivateDownloadStateViewL();
            break;
            }
            
        case ESymTorrentCmdOpenFile:
            {
            TInt index = iFilesContainer->FilesListBox()->CurrentItemIndex();
					
			if (index >= 0)
			{
				CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
            	const CSTFile* file = torrent->File(iFilesContainer->FilesListBox()->CurrentItemIndex());
            	
            	if (file->IsDownloaded())        
					static_cast<CSymTorrentAppUi*>(AppUi())->OpenFileL(file->Path());
			}	
            		
            break;
            }
            
        case ESymTorrentCmdFileDetails:
            {
            
            CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
            const CSTFile* file = torrent->File(iFilesContainer->FilesListBox()->CurrentItemIndex());
			HBufC* info = file->CreateFileInfoL();
			CleanupStack::PushL(info);	

			CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
			CleanupStack::PushL(dlg);
			dlg->PrepareLC( R_DATA_DIALOG );
			dlg->SetMessageTextL(*info);
			dlg->QueryHeading()->SetTextL(_L("File details"));
			CleanupStack::Pop(); //dlg
			dlg->RunLD();

			CleanupStack::PopAndDestroy(); //info
            break;
            }
            
        case EAknSoftkeyBack:
            {
            iAppUi->ActivateMainViewL();
            break;
            }
            
        case ESymTorrentCmdDontDownloadAnyFile:
        	{
        		CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
        		torrent->SetAllFilesSkippedL(ETrue);
	        	iFilesContainer->RebuildListL();
	        	iFilesContainer->DrawDeferred();
        	}
        	break;
        	
        case ESymTorrentCmdDownloadAllFiles:
        	{
        		CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
        		torrent->SetAllFilesSkippedL(EFalse);
	        	iFilesContainer->RebuildListL();
	        	iFilesContainer->DrawDeferred();
        	}
        	break;
        	
        case ESymTorrentCmdDontDownloadFile:
	        {
	        	TInt index = iFilesContainer->FilesListBox()->CurrentItemIndex();
	        	CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
	        	torrent->SetFileSkippedL(index, ETrue);
	        	iFilesContainer->RebuildListL();
	        	iFilesContainer->DrawDeferred();
	        }
	        break;
	        
        case ESymTorrentCmdDownloadFile:
	        {
	        	TInt index = iFilesContainer->FilesListBox()->CurrentItemIndex();
	        	CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
	        	torrent->SetFileSkippedL(index, EFalse);
	        	iFilesContainer->RebuildListL();
	        	iFilesContainer->DrawDeferred();
	        }
	        break;
	        
        case ESymTorrentCmdStopSeeding:	
    	case ESymTorrentCmdStopTorrent:
    	{    	
    		CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
			if (torrent)
				torrent->StopL();
    	}
    	break;
    	
    	case ESymTorrentCmdStartSeeding:
    	case ESymTorrentCmdStartTorrent:
    	{    	
    		CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
			if (torrent)
			{
				if (torrent->HasEnoughDiskSpaceToDownload())
				{					
					torrent->StartL();
					static_cast<CSymTorrentAppUi*>(AppUi())->ActivateDownloadStateViewL();
				}
				else
					if (iEikonEnv->QueryWinL(_L("There is not enough memory for full download. Do you want to continue?"), _L("")))
					{
						torrent->StartL();
						static_cast<CSymTorrentAppUi*>(AppUi())->ActivateDownloadStateViewL();
					}
			}
    	}
    	break;
	        
        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
}

// ---------------------------------------------------------
// CSymTorrentFilesView::HandleViewRectChange()
// ---------------------------------------------------------
//
void CSymTorrentFilesView::HandleViewRectChange()
{
    if ( iFilesContainer )
    {
        iFilesContainer->SetRect( ClientRect() );
    }
}

// ---------------------------------------------------------
// CSymTorrentFilesView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentFilesView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
	static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentFilesView);
	
    if (!iFilesContainer)
    {
        iFilesContainer = new (ELeave) CSymTorrentFilesContainer;
        iFilesContainer->SetMopParent(this);
        iFilesContainer->ConstructL( ClientRect() , iAppUi, iTorrentMgr);
        
        CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
        if (iPreviousTorrent == torrent)
        {
        	iFilesContainer->FilesListBox()->SetCurrentItemIndex(iListIndex);
        	iFilesContainer->DrawDeferred();
        }
        else
        	iPreviousTorrent = torrent;
        
        AppUi()->AddToStackL( *this, iFilesContainer );	
     }
}

// ---------------------------------------------------------
// CSymTorrentFilesView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentFilesView::DoDeactivate()
{
    if ( iFilesContainer )
    {
    	iListIndex = iFilesContainer->FilesListBox()->CurrentItemIndex();
        AppUi()->RemoveFromViewStack( *this, iFilesContainer );
    }
    
    delete iFilesContainer;
    iFilesContainer = NULL;	
}

void CSymTorrentFilesView::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
{
	TInt index = iFilesContainer->FilesListBox()->CurrentItemIndex();
	CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
	const CSTFile* file = torrent->File(iFilesContainer->FilesListBox()->CurrentItemIndex());
							
	TBool allFilesSkipped = ETrue;
	TBool allFilesDownloading = ETrue;
	TBool allFilesComplete = ETrue;
	
	switch (aResourceId)
	{
		case R_SYMTORRENT_FILE_SKIP_SUBMENU:
		{
			for (TInt i=0; i<torrent->FileCount(); i++)
        	{
        		if (torrent->File(i)->IsSkipped())
        			allFilesDownloading = EFalse;
        		else
        			allFilesSkipped = EFalse;
        		
        		if (!torrent->File(i)->IsDownloaded())
        			 allFilesComplete = EFalse;
        	}
			
			if (allFilesComplete)
			{
				aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadAnyFile, ETrue);
				aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadAllFiles, ETrue);
			}
			else
			{
				if (allFilesSkipped)
					aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadAnyFile, ETrue);
				else
					aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadAnyFile, EFalse);
				
				if (allFilesDownloading)
					aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadAllFiles, ETrue);
				else
					aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadAllFiles, EFalse);
			}								
			
			if (!file->IsDownloaded())
			{
				if (file->IsSkipped())
				{
					aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadFile, ETrue);
					aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadFile, EFalse);
				}						
				else
				{
					aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadFile, EFalse);
					aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadFile, ETrue);
				}						
			}
			else
			{
				aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadFile, ETrue);
				aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadFile, ETrue);
			}
			
							/*aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadFile, ETrue);
							aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadFile, ETrue);
							aMenuPane->SetItemDimmed(ESymTorrentCmdDontDownloadAnyFile, ETrue);
							aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadAllFiles, ETrue);*/	
		}
		break;
		
		case R_SYMTORRENT_FILESVIEW_MENU:	
		{																
			if (index >= 0)
			{
				aMenuPane->SetItemDimmed(ESymTorrentCmdFileDetails, EFalse);
				            										
				if (torrent->IsActive())
				{
					if (torrent->IsComplete())
					{
						aMenuPane->SetItemDimmed( ESymTorrentCmdStopSeeding, EFalse);
						aMenuPane->SetItemDimmed( ESymTorrentCmdStopTorrent, ETrue);
					}					
					else
					{
						aMenuPane->SetItemDimmed( ESymTorrentCmdStopTorrent, EFalse);
						aMenuPane->SetItemDimmed( ESymTorrentCmdStopSeeding, ETrue);
					}					
					
					aMenuPane->SetItemDimmed( ESymTorrentCmdStartTorrent, ETrue);
					aMenuPane->SetItemDimmed( ESymTorrentCmdStartSeeding, ETrue);
				}
				else
				{
					if (torrent->IsComplete())
					{
						aMenuPane->SetItemDimmed( ESymTorrentCmdStartSeeding, EFalse);
						aMenuPane->SetItemDimmed( ESymTorrentCmdStartTorrent, ETrue);
					}					
					else
					{
						aMenuPane->SetItemDimmed( ESymTorrentCmdStartTorrent, EFalse);
						aMenuPane->SetItemDimmed( ESymTorrentCmdStartSeeding, ETrue);
					}
										
					aMenuPane->SetItemDimmed( ESymTorrentCmdStopTorrent, ETrue);
					aMenuPane->SetItemDimmed( ESymTorrentCmdStopSeeding, ETrue);
				}    			
			}
			else            			
			{
				aMenuPane->SetItemDimmed(ESymTorrentCmdFileDetails, ETrue);	
				aMenuPane->SetItemDimmed(ESymTorrentCmdDownloadSkip, ETrue);
			}					
			
			if ((index < 0) || (!file->IsDownloaded()))
				aMenuPane->SetItemDimmed(ESymTorrentCmdOpenFile, ETrue);
			else
				aMenuPane->SetItemDimmed(ESymTorrentCmdOpenFile, EFalse);
		}
		break;

		default:
		break;
	}
	
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}
