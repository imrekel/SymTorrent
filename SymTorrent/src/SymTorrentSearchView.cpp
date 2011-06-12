/*****************************************************************************
 * Copyright (C) 2006,2007 Imre Kelényi
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
*  Name     : CSymTorrentSearchView from SymTorrentSearchView.h
*  Part of  : SymTorrent
*  Created  : 06.02.2007 by Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <SymTorrent.rsg>
#include <akntitle.h>
#include <eikbtgpc.h>
#include "SymTorrent.hrh"
#include "SymTorrentSearchView.h"
#include "SymTorrentSearchContainer.h" 
#include "STGlobal.h"
#include "STPreferences.h"
#include "STTorrentManagerSingleton.h"


// ================= MEMBER FUNCTIONS ======================

// ---------------------------------------------------------
// CSymTorrentSearchView::ConstructL
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentSearchView::ConstructL(CSTTorrentManager* aTorrentMgr, CSymTorrentAppUi* aAppUi)
{
	iTorrentMgr=aTorrentMgr;
	iAppUi=aAppUi;
    BaseConstructL( R_SYMTORRENT_SEARCHVIEW );
}

// ---------------------------------------------------------
// CSymTorrentView2::~CSymTorrentView2()
// destructor
// ---------------------------------------------------------
//
CSymTorrentSearchView::~CSymTorrentSearchView()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
    }

    delete iContainer;
}

// ---------------------------------------------------------
// TUid CSymTorrentSearchView::Id()
// 
// ---------------------------------------------------------
//
TUid CSymTorrentSearchView::Id() const
{
    return TUid::Uid(ESymTorrentSearchView);
}

// ---------------------------------------------------------
// CSymTorrentSearchView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentSearchView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {
        case EAknSoftkeyBack:
        {
            iAppUi->ActivateMainViewL();
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
// CSymTorrentSearchView::HandleViewRectChange()
// ---------------------------------------------------------
//
void CSymTorrentSearchView::HandleViewRectChange()
{
    if ( iContainer )
    {
        iContainer->SetRect( ClientRect() );
    }
}

// ---------------------------------------------------------
// CSymTorrentSearchView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentSearchView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
    static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentSearchView);
    
    if (!iContainer)
        {        
        iContainer = new (ELeave) CSymTorrentSearchContainer;
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect(), iAppUi, iTorrentMgr );
        AppUi()->AddToStackL( *this, iContainer );
        
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
		
        }
        
   static_cast<CSymTorrentAppUi*>(AppUi())->TitlePane()->SetTextToDefaultL();
}

// ---------------------------------------------------------
// CSymTorrentSearchView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentSearchView::DoDeactivate()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
    }  

    delete iContainer;
    iContainer = NULL;	
}
    
void CSymTorrentSearchView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane )
{	
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}

// End of File

