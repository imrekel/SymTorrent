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
*  Name     : CSymTorrentStatusView from SymTorrentStatusView.h
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <SymTorrent.rsg>
#include <akntitle.h>
#include <eikbtgpc.h>
#include "SymTorrent.hrh"
#include "SymTorrentStatusView.h"
#include "SymTorrentStatusContainer.h" 
#include "STGlobal.h"
#include "STPreferences.h"
#include "STTorrentManagerSingleton.h"

// ================= MEMBER FUNCTIONS ======================

// ---------------------------------------------------------
// CSymTorrentStatusView::ConstructL
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentStatusView::ConstructL(CSTTorrentManager* aTorrentMgr, CSymTorrentAppUi* aAppUi)
    {
	iTorrentMgr=aTorrentMgr;
	iAppUi=aAppUi;
    BaseConstructL( R_SYMTORRENT_STATUSVIEW );
    }

// ---------------------------------------------------------
// CSymTorrentView2::~CSymTorrentView2()
// destructor
// ---------------------------------------------------------
//
CSymTorrentStatusView::~CSymTorrentStatusView()
    {
    if ( iStatusContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iStatusContainer );
        }

    delete iStatusContainer;
    }

// ---------------------------------------------------------
// TUid CSymTorrentStatusView::Id()
// 
// ---------------------------------------------------------
//
TUid CSymTorrentStatusView::Id() const
    {
    return TUid::Uid(ESymTorrentStatusView);
    }

// ---------------------------------------------------------
// CSymTorrentStatusView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentStatusView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EAknSoftkeyBack:
            {
            iAppUi->ActivateMainViewL();
            break;
            }
        case ESymTorrentCmdDownloadstate:
            {
            iAppUi->ActivateDownloadStateViewL();
            break;
            }
        case ESymTorrentCmdTorrentFiles:
            {
            iAppUi->ActivateFilesViewL();
            break;
            }
        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CSymTorrentStatusView::HandleViewRectChange()
// ---------------------------------------------------------
//
void CSymTorrentStatusView::HandleViewRectChange()
    {
    if ( iStatusContainer )
        {
        iStatusContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CSymTorrentStatusView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentStatusView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
    static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentStatusView);
    
	if (!iStatusContainer)
	{        
	    iStatusContainer = new (ELeave) CSymTorrentStatusContainer;
	    iStatusContainer->SetMopParent(this);
	    iStatusContainer->ConstructL( ClientRect(), iAppUi, iTorrentMgr );
	    AppUi()->AddToStackL( *this, iStatusContainer );
	    
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
		
		iTorrentMgr->SetEngineStateObserverL(iStatusContainer);
	}
        
   static_cast<CSymTorrentAppUi*>(AppUi())->TitlePane()->SetTextToDefaultL();
}

// ---------------------------------------------------------
// CSymTorrentStatusView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentStatusView::DoDeactivate()
{
    if ( iStatusContainer )
    	AppUi()->RemoveFromViewStack( *this, iStatusContainer );
    
    iTorrentMgr->RemoveEngineStateObserver();
    delete iStatusContainer;
    iStatusContainer = NULL;	
}
    
void CSymTorrentStatusView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane )
{	
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}

// End of File
