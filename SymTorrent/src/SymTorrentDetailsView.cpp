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
 *  Name     : CSymTorrentDetailsView from SymTorrentDetailsView.h
 *  Part of  : SymTorrent
 *  Created  : 31.01.2006 by Imre Kelényi
 *  Copyright: 2006
 * ============================================================================
 */

// INCLUDE FILES
#include  <aknviewappui.h>
#include  <avkon.hrh>
#include <SymTorrent.rsg>
#include  "SymTorrentDetailsView.h"
#include  "SymTorrentDetailsContainer.h" 

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentDetailsView::ConstructL
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentDetailsView::ConstructL(CSTTorrentManager* aTorrentMgr, CSymTorrentAppUi* aAppUi)
    {
	iTorrentMgr=aTorrentMgr;
	iAppUi=aAppUi;
    BaseConstructL( R_SYMTORRENT_DETAILSVIEW );
    }

// ---------------------------------------------------------
// CSymTorrentView2::~CSymTorrentView2()
// destructor
// ---------------------------------------------------------
//
CSymTorrentDetailsView::~CSymTorrentDetailsView()
    {
    if ( iDetailsContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iDetailsContainer );
        }

    delete iDetailsContainer;
    }

// ---------------------------------------------------------
// TUid CSymTorrentDetailsView::Id()
// 
// ---------------------------------------------------------
//
TUid CSymTorrentDetailsView::Id() const
    {
    return TUid::Uid(ESymTorrentDetailsView);
    }

// ---------------------------------------------------------
// CSymTorrentDetailsView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentDetailsView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EAknSoftkeyOk:
            {
            iEikonEnv->InfoMsg( _L("view2 ok") );
            break;
            }
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
// CSymTorrentDetailsView::HandleViewRectChange()
// ---------------------------------------------------------
//
void CSymTorrentDetailsView::HandleViewRectChange()
    {
    if ( iDetailsContainer )
        {
        iDetailsContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CSymTorrentDetailsView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentDetailsView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentDetailsView);
    
    if (!iDetailsContainer)
        {
        iDetailsContainer = new (ELeave) CSymTorrentDetailsContainer;
        iDetailsContainer->SetMopParent(this);
        iDetailsContainer->ConstructL( ClientRect(), iAppUi, iTorrentMgr );
        AppUi()->AddToStackL( *this, iDetailsContainer );
        }
   }

// ---------------------------------------------------------
// CSymTorrentDetailsView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentDetailsView::DoDeactivate()
    {
    if ( iDetailsContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iDetailsContainer );
        }
    
    delete iDetailsContainer;
    iDetailsContainer = NULL;	
    }

// End of File

