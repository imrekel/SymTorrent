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
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/*
 * ============================================================================
 *  Name     : CSymTorrentTrackerSettingsContainer from SymTorrentProxySettingsContainer.h
 *  Part of  : SymTorrent
 *  Created  : 07.06.2007 by Imre Kelényi
 * ============================================================================
 */

#include "SymTorrentTrackerSettingsContainer.h"
#include "SymTorrentAppUi.h"
#include "SymTorrent.hrh"
#include "AccessPointSettingItem.h"
#include "FileSelectionSettingItem.h"
#include "STPreferences.h"
#include "NetworkManager.h"
#include "SymTorrentSettingsView.h"
#include <SymTorrent.rsg>
#include "SymTorrentTrackerManager.h"
#include "STTorrentManagerSingleton.h"

#include <aknnotewrappers.h>

enum TSymTorrentSettingIndex
{
    ESTSetIndexTrackerServicePort = 0,
	ESTSetIndexPieceSize = 1,
};


// ================= MEMBER FUNCTIONS =======================


CSymTorrentTrackerSettingsContainer::CSymTorrentTrackerSettingsContainer(CSymTorrentAppUi* aAppUi, CSymTorrentSettingsView* aView)
 : iAppUi(aAppUi), iView(aView)
{
	
}


CSymTorrentTrackerSettingsContainer::~CSymTorrentTrackerSettingsContainer()
{
}


void CSymTorrentTrackerSettingsContainer::ConstructL(const TRect& aRect)
{
    CreateWindowL();
	ConstructFromResourceL(R_SYMTORRENT_TRACKER_SETTING_ITEM_LIST);

	SetRect(aRect);
	ActivateL();
}


CAknSettingItem* CSymTorrentTrackerSettingsContainer::CreateSettingItemL( TInt aIdentifier )
{	
	CAknSettingItem* settingItem = NULL;	

	switch (aIdentifier)
	{
	       
		case ESTSetTrackerServicePort:
		{
			iTrackerServicePort = PREFERENCES->TrackerServicePort();
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(
				aIdentifier, iTrackerServicePort );
		}
		break;
		
		case ESTSetPieceSize:
		{
			iPieceSize = PREFERENCES->PieceSize();
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(
				aIdentifier, iPieceSize );
		}
		break;	        
	             
		default:
		break;
	}

    return settingItem;
}


void CSymTorrentTrackerSettingsContainer::EditItemL(TInt aIndex, TBool aCalledFromMenu)
{
	CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu);

	(*SettingItemArray())[aIndex]->StoreL();	
				
	CSTTorrentManager* torrentMgr = TORRENTMGR;
	CSTPreferences* prefs = PREFERENCES;
	CNetworkManager* netMgr = torrentMgr->NetworkManager();
	
	switch (aIndex)
	{
		
		case ESTSetIndexTrackerServicePort:
		{
			prefs->SetTrackerServicePort(iTrackerServicePort);
		}
		break;

		#ifdef TRACKER_BUILD
		case ESTSetIndexPieceSize:
		{		
			iAppUi->TrackerManager()->SetPieceSize(iPieceSize);	
			prefs->SetPieceSize(iPieceSize);
		}
		break;	
		#endif
		
		default:
		break;
	}
	
	(*SettingItemArray())[aIndex]->StoreL();
}

TKeyResponse CSymTorrentTrackerSettingsContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	if (aType == EEventKey) 
	{
		switch (aKeyEvent.iCode) 
		{
			case EKeyLeftArrow:
			{
				iView->SetActiveSettingPage(ESettingGeneral);
				return EKeyWasConsumed;
			}						
		
			default:;
		}
	}
	
	return CAknSettingItemList::OfferKeyEventL(aKeyEvent, aType);
}

