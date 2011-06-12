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
*  Name     : CSymTorrentMainView from SymTorrentMainView.h
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <SymTorrent.rsg>
#include <eikbtgpc.h>
#include <aknnotewrappers.h>
#include <akntitle.h> 
#include "SymTorrentMainView.h"
#include "SymTorrentMainContainer.h" 
#include "Symtorrent.hrh"
#include "STGlobal.h"
#include "STTorrent.h"
#include "SymTorrent.hrh"
#include "STPreferences.h"
#include "STTorrentManagerSingleton.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentMainView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentMainView::ConstructL(CSTTorrentManager* aTorrentMgr, 
	CSymTorrentAppUi* aAppUi)
{
	iAppUi=aAppUi;
	iTorrentMgr=aTorrentMgr;	
    BaseConstructL( R_SYMTORRENT_MAINVIEW );
	iLastItemIndex=-1;
}

// ---------------------------------------------------------
// CSymTorrentMainView::~CSymTorrentMainView()
// destructor
// ---------------------------------------------------------
//
CSymTorrentMainView::~CSymTorrentMainView()
{
    if ( iMainContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iMainContainer );
    }
    
    delete iMainContainer;
    iMainContainer = NULL;
}

// ---------------------------------------------------------
// TUid CSymTorrentMainView::Id()
//
// ---------------------------------------------------------
//
TUid CSymTorrentMainView::Id() const
{
    return TUid::Uid(ESymTorrentMainView);
}

// ---------------------------------------------------------
// CSymTorrentMainView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentMainView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
        {            
		case ESymTorrentCmdDetails:
			{
				TInt currentitem=iMainContainer->ListBox()->CurrentItemIndex();
				
				if (currentitem > -1)
				{
					iAppUi->SetSelectedTorrentIndex(iMainContainer->ListBox()->CurrentItemIndex());
					iAppUi->ActivateDownloadStateViewL();
				}
				else
				{
					CAknWarningNote* note = new(ELeave) CAknWarningNote;
					note->ExecuteLD(_L("No torrent selected!"));
				}
				break;
			}
			
		case ESymTorrentCmdRemoveTorrent:
			{
				TInt currentitem =
					iMainContainer->ListBox()->CurrentItemIndex();
				
				if (currentitem >= 0)
					if (iEikonEnv->QueryWinL(_L("Remove torrent?"), _L("")))
					{
						TBool deleteIncompleteFiles = EFalse;
						
						if (TORRENTMGR->Torrent(currentitem)->BytesDownloaded() > 0 && !TORRENTMGR->Torrent(currentitem)->IsComplete())
							deleteIncompleteFiles = iEikonEnv->QueryWinL(_L("Delete incomplete files?"), _L(""));
						
						TORRENTMGR->RemoveTorrentL(currentitem, deleteIncompleteFiles);
					}
				break;				
			}
		
		case ESymTorrentCmdStopSeeding:	
    	case ESymTorrentCmdStopTorrent:
    	{
    		if (iMainContainer && (iMainContainer->ListBox()->CurrentItemIndex() >= 0))
			{
				CSTTorrent* torrent = TORRENTMGR->Torrent(iMainContainer->ListBox()->CurrentItemIndex());
				torrent->StopL();
			}
    	}
    	break;
    	
    	case ESymTorrentCmdStartSeeding:
    	case ESymTorrentCmdStartTorrent:
    	{
    		if (iMainContainer && (iMainContainer->ListBox()->CurrentItemIndex() >= 0))
			{
				CSTTorrent* torrent = TORRENTMGR->Torrent(iMainContainer->ListBox()->CurrentItemIndex());
				
				if (torrent->HasEnoughDiskSpaceToDownload())
					torrent->StartL();
				else
					if (iEikonEnv->QueryWinL(_L("There is not enough memory for full download. Do you want to continue?"), _L("")))
						torrent->StartL();
			}
    	}
    	break;
            
        default:
            {
			//---OFFLINETEST---for (TInt i=0; i<100; i++);
			//---OFFLINETEST---iMainContainer->ModifyTorrentL(0,_L("file"),1,1,0,5,7,EUnused);

            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
}

// ---------------------------------------------------------
// CSymTorrentMainView::HandleViewRectChange()
// ---------------------------------------------------------
//
void CSymTorrentMainView::HandleViewRectChange()
{
    if ( iMainContainer )
        {
        iMainContainer->SetRect( ClientRect() );
        }
}

// ---------------------------------------------------------
// CSymTorrentMainView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentMainView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
	static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentMainView);
	
    if (!iMainContainer)
    {
        iMainContainer = new (ELeave) CSymTorrentMainContainer;
        iMainContainer->SetMopParent(this);
        iMainContainer->ConstructL( ClientRect() , iAppUi, iTorrentMgr);
        AppUi()->AddToStackL( *this, iMainContainer );
        
        if (PREFERENCES->RightSoftkeyMode() == ESTSoftkeyHide)
        {
    		Cba()->SetCommandL(2, ESymTorrentCmdHide, KLitHideButtonText);        	
			Cba()->DrawDeferred(); 	
        }
        else         	
    	{
    		Cba()->SetCommandL(2, EAknSoftkeyExit, KLitExitButtonText);        	
			Cba()->DrawDeferred();
    	}
    	
    	iTorrentMgr->AddTorrentObserverL(iMainContainer);  
	    for (TInt i=0; i<iTorrentMgr->TorrentCount(); i++)
	    {
	    	iMainContainer->InsertTorrentL(iTorrentMgr->Torrent(i), i);
	    }
    }

	if (iLastItemIndex != -1)
		iMainContainer->ListBox()->SetCurrentItemIndex(iLastItemIndex);
	
	static_cast<CSymTorrentAppUi*>(AppUi())->TitlePane()->SetTextToDefaultL();
}

// ---------------------------------------------------------
// CSymTorrentMainView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentMainView::DoDeactivate()
{
    if ( iMainContainer )
    {
		iLastItemIndex=iMainContainer->ListBox()->CurrentItemIndex();
        AppUi()->RemoveFromViewStack( *this, iMainContainer );
    }
     
	iTorrentMgr->RemoveTorrentObserver(iMainContainer);	
    
    delete iMainContainer;
    iMainContainer = NULL;	
}


// ---------------------------------------------------------
// CSymTorrentMainView::DynInitMenuPaneL()
// Sets the options menu
// ---------------------------------------------------------
//
void CSymTorrentMainView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane )
{
	if ( aResourceId == R_SYMTORRENT_MAINVIEW_MENU )
	{
		if (iMainContainer && (iMainContainer->ListBox()->CurrentItemIndex() >= 0))
		{
			aMenuPane->SetItemDimmed( ESymTorrentCmdDetails, EFalse);
			aMenuPane->SetItemDimmed( ESymTorrentCmdRemoveTorrent, EFalse);
			
			CSTTorrent* torrent = TORRENTMGR->Torrent(iMainContainer->ListBox()->CurrentItemIndex());
			
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
			aMenuPane->SetItemDimmed( ESymTorrentCmdStartSeeding, ETrue);
			aMenuPane->SetItemDimmed( ESymTorrentCmdStopSeeding, ETrue);
			aMenuPane->SetItemDimmed( ESymTorrentCmdStopTorrent, ETrue);
			aMenuPane->SetItemDimmed( ESymTorrentCmdStartTorrent, ETrue);
			aMenuPane->SetItemDimmed( ESymTorrentCmdDetails, ETrue);
			aMenuPane->SetItemDimmed( ESymTorrentCmdRemoveTorrent, ETrue);
		}		
	}
	
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}
