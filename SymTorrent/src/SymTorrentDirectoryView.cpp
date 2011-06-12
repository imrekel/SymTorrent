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
*  Name     : CSymTorrentDirectoryView from SymTorrentDirectoryView.h
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
#include "SymTorrentDirectoryView.h"
#include "SymTorrentDirectoryContainer.h" 
#include "STGlobal.h"
#include "STPreferences.h"
#include "STTorrentManagerSingleton.h"
#include <aknnotewrappers.h>


// ================= MEMBER FUNCTIONS ======================

// ---------------------------------------------------------
// CSymTorrentDirectoryView::ConstructL
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentDirectoryView::ConstructL(CSTTorrentManager* aTorrentMgr, CSymTorrentAppUi* aAppUi)
{
	iTorrentMgr=aTorrentMgr;
	iAppUi=aAppUi;
    BaseConstructL( R_SYMTORRENT_DIRECTORYVIEW );
}

// ---------------------------------------------------------
// CSymTorrentView2::~CSymTorrentView2()
// destructor
// ---------------------------------------------------------
//
CSymTorrentDirectoryView::~CSymTorrentDirectoryView()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
    }

    delete iContainer;
}

// ---------------------------------------------------------
// TUid CSymTorrentDirectoryView::Id()
// 
// ---------------------------------------------------------
//
TUid CSymTorrentDirectoryView::Id() const
{
    return TUid::Uid(ESymTorrentDirectoryView);
}

// ---------------------------------------------------------
// CSymTorrentDirectoryView::HandleCommandL(TInt aCommand)
// takes care of view command handling
// ---------------------------------------------------------
//
void CSymTorrentDirectoryView::HandleCommandL(TInt aCommand)
{   
    switch ( aCommand )
    {
        case EAknSoftkeyBack:
        {
            iAppUi->ActivateMainViewL();
            break;
        }
        
    	case ESymTorrentCmdDirectoryViewOpen:
    	{
    	//	(new(ELeave) CAknInformationNote)->ExecuteLD(_L("Pákpák!"));
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
// CSymTorrentDirectoryView::HandleViewRectChange()
// ---------------------------------------------------------
//
void CSymTorrentDirectoryView::HandleViewRectChange()
{
    if ( iContainer )
    {
        iContainer->SetRect( ClientRect() );
    }
}

// ---------------------------------------------------------
// CSymTorrentDirectoryView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CSymTorrentDirectoryView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
    static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentDirectoryView);
    
    if (!iContainer)
        {        
        iContainer = new (ELeave) CSymTorrentDirectoryContainer;
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
// CSymTorrentDirectoryView::DoDeactivate()
// 
// ---------------------------------------------------------
//
void CSymTorrentDirectoryView::DoDeactivate()
{
    if ( iContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iContainer );
    }  

    delete iContainer;
    iContainer = NULL;	
}
    
void CSymTorrentDirectoryView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane )
{	
	AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
}

// End of File

