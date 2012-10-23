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
*  Name     : CSymTorrentProxySettingsContainer from SymTorrentProxySettingsContainer.h
*  Part of  : SymTorrent
*  Created  : 07.06.2007 by Imre Kelényi
* ============================================================================
*/

#include "SymTorrentProxySettingsContainer.h"
#include "SymTorrentAppUi.h"
#include "SymTorrent.hrh"
#include "STPreferences.h"
#include "NetworkManager.h"
#include "SymTorrentSettingsView.h"
#include <SymTorrent.rsg>
#include "STTorrentManagerSingleton.h"

#include <aknnotewrappers.h>

enum TSymTorrentSettingIndex
{
    ESTSetIndexIncomingConnections = 0,
    ESTSetIndexProxyAddress = 1,   
    ESTSetIndexProxyConnectionPort = 2,
    ESTSetIndexProxyServicePort = 3,
};


// ================= MEMBER FUNCTIONS =======================


CSymTorrentProxySettingsContainer::CSymTorrentProxySettingsContainer(CSymTorrentAppUi* aAppUi, CSymTorrentSettingsView* aView)
 : iAppUi(aAppUi), iView(aView)
{	
}


CSymTorrentProxySettingsContainer::~CSymTorrentProxySettingsContainer()
{
}


void CSymTorrentProxySettingsContainer::ConstructL(const TRect& aRect)
{
    CreateWindowL();
	ConstructFromResourceL(R_SYMTORRENT_PROXY_SETTING_ITEM_LIST);

	SetRect(aRect);
	ActivateL();
}


CAknSettingItem* CSymTorrentProxySettingsContainer::CreateSettingItemL( TInt aIdentifier )
{	
	CAknSettingItem* settingItem = NULL;	

	switch (aIdentifier)
	{	       
		case ESTSetIncomingConnections:
		{
    		iIncomingConnections = TInt(PREFERENCES->IncomingConnectionsMode()); 
			settingItem = new (ELeave) CAknEnumeratedTextPopupSettingItem(
				aIdentifier, iIncomingConnections);
		}
		break;
	        
		case ESTSetProxyAddress:
		{
     		iProxyAddress.Input(PREFERENCES->ProxyHostName()); 
       		settingItem = new (ELeave) CAknIpFieldSettingItem(
       			aIdentifier, iProxyAddress);
		}
 		break;
	 		        
		case ESTSetProxyConnectionPort:
		{
    		iProxyConnectionPort = PREFERENCES->ProxyConnectionPort();
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(
        		aIdentifier, iProxyConnectionPort);
		}
		break;
	        
		case ESTSetProxyServicePort:
		{    	
			iProxyServicePort = PREFERENCES->ProxyServicePort();
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(
        		aIdentifier, iProxyServicePort);
		}        	
		break;	        
	             
		default:
			break;
	}

    return settingItem;
}


void CSymTorrentProxySettingsContainer::EditItemL(TInt aIndex, TBool aCalledFromMenu)
{
	CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu);

	(*SettingItemArray())[aIndex]->StoreL();
	
	//TIncomingConnectionsMode incomingConnectionsMode = 
	//		TIncomingConnectionsMode(iIncomingConnections);
				
	CSTPreferences* prefs = PREFERENCES;
	
	switch (aIndex)
	{		
		case ESTSetIndexIncomingConnections:
		{							
			//prefs->SetIncomingConnectionsMode(incomingConnectionsMode);					
		}
		break;
	
		case ESTSetIndexProxyAddress:
		{
			TBuf<100> proxyAddressBuf;
			iProxyAddress.Output(proxyAddressBuf);
			prefs->SetProxyAddressL(proxyAddressBuf);														
		}
		break;
		
		case ESTSetIndexProxyConnectionPort:
		{
			prefs->SetProxyConnectionPort(iProxyConnectionPort);		
		}
		break;
		
		case ESTSetIndexProxyServicePort:
		{			
			prefs->SetProxyServicePort(iProxyServicePort);
		}
		break;		
		
		default:
		break;
	}
	
	(*SettingItemArray())[aIndex]->StoreL();
}

TKeyResponse CSymTorrentProxySettingsContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
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
			
			#ifdef TRACKER_BUILD
			case EKeyRightArrow:
			{
				iView->SetActiveSettingPage(ESettingTracker);
				return EKeyWasConsumed;
			}
			#endif
		
			default:;
		}
	}
	
	return CAknSettingItemList::OfferKeyEventL(aKeyEvent, aType);
}

