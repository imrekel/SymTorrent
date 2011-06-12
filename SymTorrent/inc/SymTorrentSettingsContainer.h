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
*  Name     : CSymTorrentSettingsContainer from SymTorrentSettingsContainer.h
*  Part of  : SymTorrent
*  Created  : 13.03.2006 by Péter Ekler
*  Description:
*     Declares the settings container for application.
*  Version  :
*  Copyright: 2006
* ============================================================================
*/

#ifndef SYMTORRENTSETTINGSCONTAINER
#define SYMTORRENTSETTINGSCONTAINER

// INCLUDES
#include <AknSettingItemList.h>
#include <aknsettingpage.h> 
#include "STDefs.h"

// FORWARD DECLARATIONS
class CSymTorrentAppUi;
class CAccessPointSettingItem;
class CFileSelectionSettingItem;
class CSymTorrentSettingsView;

// CLASS DECLARATION

class CSymTorrentSettingsContainer: public CAknSettingItemList 
{
    public:
        /**
        * EPOC default constructor.
        * @param aAppUi SymTorrentAppUi.
        */
    	CSymTorrentSettingsContainer(CSymTorrentAppUi* aAppUi, CSymTorrentSettingsView* aView);
    	
        /**
        * EPOC default constructor.
        */
    	void ConstructL(const TRect& aRect);
    	
    	~CSymTorrentSettingsContainer();
    	
		/**
        * From CAknSettingItemList
        */
        CAknSettingItem* CreateSettingItemL(TInt identifier);

        /**
        * From CAknSettingItemList 
        */
        void EditItemL(TInt aIndex, TBool aCalledFromMenu);
        
    private:
    
    	TKeyResponse OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType);
        
    private:
    
    	CAccessPointSettingItem*	iAccesPointSettingItem;
    	CFileSelectionSettingItem*	iFileSelectionSettingItem;
    
    	CSymTorrentAppUi*			iAppUi;
		
		TInt						iIncomingConnections;
		TInt						iUploadEnabled;
		TInt						iRightSoftkeyHide;
		TInt						iIncomingPort;
		
		TInt						iStartupHashCheck;
		
		TInt						iCloseConnectionAfterDownload;
		
		TInt						iSubpieceSizeType;
		
		TInt						iSubpieceSize;
		
		TInt32						iApId;
		TBuf<128>					iApName;
		TFileName					iDownloadFolder;
		
		CSymTorrentSettingsView* 	iView;
};

#endif

// End of File
