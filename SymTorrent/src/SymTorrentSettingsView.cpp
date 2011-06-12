/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
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
 * along with SymTorrent; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/*
* ============================================================================
*  Name     : CSymTorrentSettingsView from SymTorrentSettingsView.h
*  Part of  : SymTorrent
*  Created  : 13.03.2006 by Péter Ekler
* ============================================================================
*/

// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <SymTorrent.rsg>
#include <akntabgrp.h>
#include <akntitle.h> 
#include "SymTorrent.hrh"
#include "SymTorrentSettingsView.h"
#include "SymTorrentSettingsContainer.h"
#include "SymTorrentProxySettingsContainer.h"
#ifdef TRACKER_BUILD
#include "SymTorrentTrackerSettingsContainer.h"
#endif
#include "SymTorrentAppUi.h"

// ================= MEMBER FUNCTIONS =======================

CSymTorrentSettingsView::CSymTorrentSettingsView()
  : iActiveSettingPage(ESettingGeneral)
 {      	 	
 }

// ---------------------------------------------------------
// CSymTorrentSettingsView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentSettingsView::ConstructL()
{
	BaseConstructL( R_SYMTORRENT_SETTINGSVIEW );
	
	iNaviTabDecorator = static_cast<CSymTorrentAppUi*>(AppUi())->NaviPane()->CreateTabGroupL();
	
	iNaviTabGroup = (CAknTabGroup*) iNaviTabDecorator->DecoratedControl();
	
	iNaviTabGroup->SetTabFixedWidthL(KTabWidthWithTwoLongTabs);

	iNaviTabGroup->AddTabL(ESymTorrentSettingsGeneralTab, _L("General"));
	//iNaviTabGroup->AddTabL(ESymTorrentSettingsProxyTab, _L("Proxy"));
	#ifdef TRACKER_BUILD
		iNaviTabGroup->AddTabL(ESymTorrentSettingsTrackerTab, _L("Tracker"));
	#endif
}

// ---------------------------------------------------------
// CSymTorrentSettingsView::~CSymTorrentSettingsView()
// ---------------------------------------------------------
//
CSymTorrentSettingsView::~CSymTorrentSettingsView()
{
    if ( iSettingsContainer )
    	AppUi()->RemoveFromViewStack( *this, iSettingsContainer );

    delete iSettingsContainer;
    delete iNaviTabDecorator;
}

// ---------------------------------------------------------
// TUid CSymTorrentSettingsView::Id()
// ---------------------------------------------------------
//
TUid CSymTorrentSettingsView::Id() const
    {
    return TUid::Uid(ESymTorrentSettingsView);
    }

// ---------------------------------------------------------
// CSymTorrentSettingsView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
//
void CSymTorrentSettingsView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EAknSoftkeySelect:
            {
            iSettingsContainer->EditItemL(
				iSettingsContainer->ListBox()->CurrentItemIndex(), EFalse);
            break;
            }
        case EAknSoftkeyCancel:
            {			
			#ifdef TRACKER_BUILD
			if (static_cast<CSymTorrentAppUi*>(AppUi())->InTorrentMaker())
				static_cast<CSymTorrentAppUi*>(AppUi())->ActivateTorrentMakerFileListView();
			else if (static_cast<CSymTorrentAppUi*>(AppUi())->InTracker())
				static_cast<CSymTorrentAppUi*>(AppUi())->ActivateTrackerTorrentListViewL();
			else
				static_cast<CSymTorrentAppUi*>(AppUi())->ActivateMainViewL();
			#else
			static_cast<CSymTorrentAppUi*>(AppUi())->ActivateMainViewL();
			#endif
			
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
// CSymTorrentSettingsView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CSymTorrentSettingsView::HandleViewRectChange()
{
    if ( iSettingsContainer )
    {
        iSettingsContainer->SetRect( ClientRect() );
    }
}

// ---------------------------------------------------------
// CSymTorrentSettingsView::DoActivateL(...)
// ---------------------------------------------------------
//
void CSymTorrentSettingsView::DoActivateL( const TVwsViewId& aPrevViewId,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
{
    static_cast<CSymTorrentAppUi*>(AppUi())->SetCurrentViewId(ESymTorrentSettingsView);
    
    if (!iSettingsContainer)
    {
		iPrevViewId = aPrevViewId;
        SetActiveSettingPage(iActiveSettingPage);
        
        static_cast<CSymTorrentAppUi*>(AppUi())->TitlePane()->SetTextL(_L("Settings"));
        
		#ifdef TRACKER_BUILD
        	static_cast<CSymTorrentAppUi*>(AppUi())->NaviPane()->PushL(*iNaviTabDecorator);
		#endif
	}
}

// ---------------------------------------------------------
// CSymTorrentSettingsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CSymTorrentSettingsView::DoDeactivate()
{
    if ( iSettingsContainer )
    {
        AppUi()->RemoveFromViewStack( *this, iSettingsContainer );
        
        delete iSettingsContainer;
    	iSettingsContainer = NULL;
    	
		#ifdef TRACKER_BUILD
    		static_cast<CSymTorrentAppUi*>(AppUi())->NaviPane()->Pop(iNaviTabDecorator);
		#endif
    }    
}

    
void CSymTorrentSettingsView::SetActiveSettingPage(TSymTorrentSettingType aSettingPage)
{
	iActiveSettingPage = aSettingPage;			
	
	if (static_cast<CSymTorrentAppUi*>(AppUi())->CurrentViewId() == ESymTorrentSettingsView)
	{
		
		TInt tabIndex = 0;
		
		CAknSettingItemList* prevContainer = NULL;
		if (iSettingsContainer)
		{
			 prevContainer = iSettingsContainer;
			 CleanupStack::PushL(prevContainer);
		}
	
		switch (aSettingPage)
		{
			case ESettingGeneral:
			{
				iSettingsContainer = new (ELeave) CSymTorrentSettingsContainer(static_cast<CSymTorrentAppUi*>(AppUi()), this);
				static_cast<CSymTorrentSettingsContainer*>(iSettingsContainer)->ConstructL( ClientRect() );
				tabIndex = 0;
			}
			break;
			
			/*case ESettingProxy:
			{
				iSettingsContainer = new (ELeave) CSymTorrentProxySettingsContainer(static_cast<CSymTorrentAppUi*>(AppUi()), this);
				static_cast<CSymTorrentProxySettingsContainer*>(iSettingsContainer)->ConstructL( ClientRect() );
				tabIndex = 1;
			}
			break;*/
			
			#ifdef TRACKER_BUILD
			case ESettingTracker:
			{
				iSettingsContainer = new (ELeave) CSymTorrentTrackerSettingsContainer(static_cast<CSymTorrentAppUi*>(AppUi()), this);
				static_cast<CSymTorrentTrackerSettingsContainer*>(iSettingsContainer)->ConstructL( ClientRect() );
				tabIndex = 1;
			}
			break;
			#endif
			
			default:
				iSettingsContainer = NULL;
			break;
		}
        
		#ifdef TRACKER_BUILD
			iNaviTabGroup->SetActiveTabByIndex(tabIndex);
		#endif
		
		if (prevContainer)
		{
			AppUi()->RemoveFromViewStack(*this, prevContainer);
			CleanupStack::PopAndDestroy(); // prevContainer;
		}
		
		if (iSettingsContainer)
		{
			iSettingsContainer->SetMopParent(this);		
	        AppUi()->AddToStackL( *this, iSettingsContainer );
		}
	}
}

// End of File

