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
*  Name     : CSymTorrentDownloadStateView from SymTorrentDownloadStateView.h
*  Part of  : SymTorrent
*  Created  : 06.03.2006 by Péter Ekler
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <eikmenup.h>
#include <SymTorrent.rsg>
#include "SymTorrent.hrh"
#include "SymTorrentDownloadStateView.h"
#include "SymTorrentDownloadStateContainer.h" 
#include "STTorrent.h"
#include "STTorrentManagerSingleton.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentDownloadStateView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateView::ConstructL(CSTTorrentManager* aTorrentMgr, CSymTorrentAppUi* aAppUi)
    {
	iTorrentMgr=aTorrentMgr;
	iAppUi=aAppUi;
    BaseConstructL( R_SYMTORRENT_DOWNLOADSTATEVIEW );
    }

// ---------------------------------------------------------
// CSymTorrentDownloadStateView::~CSymTorrentDownloadStateView()
// destructor
// ---------------------------------------------------------
//
CSymTorrentDownloadStateView::~CSymTorrentDownloadStateView()
    {
    if ( iDownloadStateContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iDownloadStateContainer );
        }

    delete iDownloadStateContainer;
    }

// ---------------------------------------------------------
// TUid CSymTorrentDownloadStateView::Id()
// 
// ---------------------------------------------------------
//
TUid CSymTorrentDownloadStateView::Id() const
    {
    return TUid::Uid(ESymTorrentDownloadStateView);
    }

// ---------------------------------------------------------
// CSymTorrentDownloadStateView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {
        case EAknSoftkeyBack:
        {
			//iDownloadStateContainer->IncreasePercent();
            iAppUi->ActivateMainViewL();
            break;
        }
        
        case ESymTorrentCmdDetailsFromTorrentDetails:
        {
            iAppUi->ActivateTorrentDetailViewL();
            break;
        }
        
        case ESymTorrentCmdTorrentFiles:
        {
            iAppUi->ActivateFilesViewL();
            break;
        }
            
        case ESymTorrentCmdStopSeeding:	
    	case ESymTorrentCmdStopTorrent:
    	{    	
			CSTTorrent* torrent = 
				TORRENTMGR->Torrent(((CSymTorrentAppUi*)iAppUi)->SelectedTorrentIndex());
			if (torrent)
				torrent->StopL();
    	}
    	break;
    	
    	case ESymTorrentCmdStartSeeding:
    	case ESymTorrentCmdStartTorrent:
    	{
    	
			CSTTorrent* torrent = 
				TORRENTMGR->Torrent(((CSymTorrentAppUi*)iAppUi)->SelectedTorrentIndex());
				
			if (torrent)
			{
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
            AppUi()->HandleCommandL( aCommand );
            break;
        }
    }
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateView::HandleViewRectChange()
{
    if ( iDownloadStateContainer )
    {
        iDownloadStateContainer->SetRect( ClientRect() );
        iDownloadStateContainer->DrawDeferred();
    }
}
    
void CSymTorrentDownloadStateView::Redraw()
{
	if ( iDownloadStateContainer )
    {
        iDownloadStateContainer->DrawDeferred();
    }
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
    static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentDownloadStateView);
    
    if (!iDownloadStateContainer)
    {
        iDownloadStateContainer = new (ELeave) CSymTorrentDownloadStateContainer;
        iDownloadStateContainer->SetMopParent(this);
        iDownloadStateContainer->ConstructL( ClientRect(), iAppUi, iTorrentMgr );
        AppUi()->AddToStackL( *this, iDownloadStateContainer );
        
        iTorrentMgr->AddTorrentObserverL(iDownloadStateContainer, iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex()));
    }
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateView::DoDeactivate()
    {
    if ( iDownloadStateContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iDownloadStateContainer );
        }
        
    iTorrentMgr->RemoveTorrentObserver(iDownloadStateContainer);
    
    delete iDownloadStateContainer;
    iDownloadStateContainer = NULL;
    }
    
    
void CSymTorrentDownloadStateView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane )
{
	if ( aResourceId == R_SYMTORRENT_DOWNLOADSTATEVIEW_MENU )
	{
		CSTTorrent* torrent = 
				TORRENTMGR->Torrent(((CSymTorrentAppUi*)iAppUi)->SelectedTorrentIndex());
		if (torrent)
		{									
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

// End of File

