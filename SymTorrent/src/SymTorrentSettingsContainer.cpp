/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
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
*  Name     : CSymTorrentSettingsContainer from SymTorrentSettingsContainer.h
*  Part of  : SymTorrent
*  Created  : 13.03.2006 by Péter Ekler
* ============================================================================
*/

#include "SymTorrentSettingsContainer.h"
#include "SymTorrentAppUi.h"
#include "SymTorrent.hrh"
#include "AccessPointSettingItem.h"
#include "FileSelectionSettingItem.h"
#include "SymTorrentSettingsView.h"
#include "STPreferences.h"
#include "NetworkManager.h"
#include <SymTorrent.rsg>
#ifdef EKA2
//	#include "TorrentMaker.h"
#endif
#include "STTorrentManagerSingleton.h"

#include <aknnotewrappers.h>

enum TSymTorrentSettingIndex
{
	ESTSetIndexDownloadFolder = 0,
	ESTSetIndexConnection = 1,
	ESTSetIndexRightSoftkey = 2,    
    ESTSetIndexIncomingPort = 3,
    ESTSetIndexStartupHashCheck = 4,
    ESTSetIndexCloseConnection = 5,
    ESTSetIndexSubpieceSizeType = 6,
    ESTSetIndexSubpieceSize = 7
};

enum TSymTorrentSettingIndexInTorrentMaker
{
	ESTSetInMakerIndexPieceSize = 0
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentSettingsContainer::CSymTorrentSettingsContainer(CSymTorrentAppUi* aAppUi)
// ---------------------------------------------------------
//
CSymTorrentSettingsContainer::CSymTorrentSettingsContainer(CSymTorrentAppUi* aAppUi, CSymTorrentSettingsView* aView)
 : iAppUi(aAppUi), iView(aView)
{
	
}

CSymTorrentSettingsContainer::~CSymTorrentSettingsContainer()
{
}

// ---------------------------------------------------------
// CSymTorrentSettingsContainer::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CSymTorrentSettingsContainer::ConstructL(const TRect& aRect)
{
    CreateWindowL();
	ConstructFromResourceL(R_SYMTORRENT_SETTING_ITEM_LIST);

	SetRect(aRect);
	ActivateL();
}

// ---------------------------------------------------------
// CSymTorrentSettingsContainer::CreateSettingItemL( TInt aIdentifier )
// From CAknSettingItemList
// ---------------------------------------------------------
//
CAknSettingItem* CSymTorrentSettingsContainer::CreateSettingItemL( TInt aIdentifier )
{	
	CAknSettingItem* settingItem = NULL;	

	switch (aIdentifier)
	{
		case ESTSetDownloadFolder:
		{
    		iDownloadFolder = PREFERENCES->DownloadPath();
			iFileSelectionSettingItem = new (ELeave) CFileSelectionSettingItem(
				aIdentifier, iDownloadFolder, ECFDDialogTypeSave);
	            
			settingItem = iFileSelectionSettingItem;
		}
		break;
	    
		case ESTSetConnection:
		{    	    
    		iApId = TInt32(PREFERENCES->AccessPointId());
			iAccesPointSettingItem = new (ELeave) CAccessPointSettingItem(
				aIdentifier, iApId, iApName);
	            
			settingItem = iAccesPointSettingItem;
		}
		break;
	        
		case ESTSetRightSoftkey:
		{    	
    		TSTSoftkeyMode softkey = PREFERENCES->RightSoftkeyMode();
    		if (softkey == ESTSoftkeyHide)
    			iRightSoftkeyHide = ETrue;
    		else
    			iRightSoftkeyHide = EFalse;
    		settingItem = new (ELeave) CAknBinaryPopupSettingItem(
    			aIdentifier, iRightSoftkeyHide);

		}
		break;	       	
	        
		case ESTSetIncomingPort:
		{    	
    		iIncomingPort = PREFERENCES->IncomingPort();
			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(
        		aIdentifier, iIncomingPort);
		}        	
		break;
		
		case ESTSetStartupHashCheck:
		{    
			iStartupHashCheck = PREFERENCES->StartupHashCheck();		 
    		settingItem = new (ELeave) CAknBinaryPopupSettingItem(
    			aIdentifier, iStartupHashCheck);
		}
		break;		 
		
	/*	case ESTSetCloseConnection:
		{    
			iCloseConnectionAfterDownload = PREFERENCES->CloseConnectionAfterDownload();		 
    		settingItem = new (ELeave) CAknBinaryPopupSettingItem(
    			aIdentifier, iCloseConnectionAfterDownload);
		}
		break;	
		
		case ESTSetSubpieceSizeType:
		{    
			iSubpieceSizeType = PREFERENCES->SubpieceSize();
			if (iSubpieceSizeType != 0)
				iSubpieceSizeType = 1;
			
			settingItem = new (ELeave) CAknBinaryPopupSettingItem(
			    aIdentifier, iSubpieceSizeType); 		
		}
		break;	
		
		case ESTSetSubpieceSize:
		{
			iSubpieceSize = PREFERENCES->SubpieceSize() / 1000;
			if (iSubpieceSize == 0)
				iSubpieceSize = 16;

			settingItem = new (ELeave) CAknIntegerEdwinSettingItem(
		       aIdentifier, iSubpieceSize);
			
			if (PREFERENCES->SubpieceSize() == 0)
				settingItem->SetHidden(ETrue);				
		}
		break;*/
		
		default:
			break;
	}

    return settingItem;
}

// ---------------------------------------------------------
// CSymTorrentSettingsContainer::EditItemL(TInt aIndex, TBool aCalledFromMenu)
// From CAknSettingItemList
// ---------------------------------------------------------
//
void CSymTorrentSettingsContainer::EditItemL(TInt aIndex, TBool aCalledFromMenu)
{
	TInt switchIndex = aIndex;

	CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu);


	if (switchIndex != ESTSetIndexDownloadFolder)
		(*SettingItemArray())[aIndex]->StoreL();
	
//	TIncomingConnectionsMode incomingConnectionsMode = 
	//			TIncomingConnectionsMode(iIncomingConnections);
				
	CSTTorrentManager* torrentMgr = TORRENTMGR;
	CSTPreferences* prefs = PREFERENCES;
	
	switch (switchIndex)
	{
		case ESTSetIndexDownloadFolder:
		{		
			prefs->SetDownloadPathL(iDownloadFolder);
			HandleChangeInItemArrayOrVisibilityL();
		}
		break;
		
		case ESTSetIndexConnection:
		{	
			prefs->SetAccessPointL(iApName, iApId);					
			HandleChangeInItemArrayOrVisibilityL(); 			
		}
		break;
		
		case ESTSetIndexRightSoftkey:
		{
			if (iRightSoftkeyHide)
				prefs->SetRightSoftkeyMode(ESTSoftkeyHide);
			else
				prefs->SetRightSoftkeyMode(ESTSoftkeyExit);
		}
		break;
		
		case ESTSetIndexIncomingPort:
		{	
			prefs->SetIncomingPort(iIncomingPort);
		}
		break;
		
		case ESTSetIndexStartupHashCheck:
		{
			if (iStartupHashCheck)
				prefs->SetStartupHashCheck(ETrue);
			else
				prefs->SetStartupHashCheck(EFalse);
		}
		break;
		
		case ESTSetIndexCloseConnection:
		{
			if (iCloseConnectionAfterDownload)
				prefs->SetCloseConnectionAfterDownload(ETrue);
			else
				prefs->SetCloseConnectionAfterDownload(EFalse);
		}
		break;
		
	/*	case ESTSetIndexSubpieceSizeType:
		{
			if (iSubpieceSizeType)
			{
				if (prefs->SubpieceSize() == 0)
				{
					prefs->SetSubpieceSize(16*1000);
					iSubpieceSize = 16;
					(*SettingItemArray())[ESTSetIndexSubpieceSize]->SetHidden(EFalse);					
					HandleChangeInItemArrayOrVisibilityL();
					//ListBox()->SetCurrentItemIndex(ESTSetIndexSubpieceSize);
				}					
			}								
			else
			{
				prefs->SetSubpieceSize(0);
				(*SettingItemArray())[ESTSetIndexSubpieceSize]->SetHidden(ETrue);
				HandleChangeInItemArrayOrVisibilityL();
			}
				
		}
		break;
		
		case ESTSetIndexSubpieceSize:
		{
			prefs->SetSubpieceSize(iSubpieceSize*1000);
		}*/
		
		default:
		break;
	}
	
	(*SettingItemArray())[aIndex]->StoreL();
}

TKeyResponse CSymTorrentSettingsContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	#ifdef TRACKER_BUILD
	if (aType == EEventKey) 
	{
		switch (aKeyEvent.iCode) 
		{
			case EKeyRightArrow:
			{
				iView->SetActiveSettingPage(ESettingTracker);
				return EKeyWasConsumed;
			}
		
			default:;
		}
	}
	#endif
	
	return CAknSettingItemList::OfferKeyEventL(aKeyEvent, aType);
}
