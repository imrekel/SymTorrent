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
*  Created  : 09.06.2007 by Imre Kelényi
* ============================================================================
*/

#ifndef SYMTORRENT_PROXY_SETTINGS_CONTAINER_H__
#define SYMTORRENT_PROXY_SETTINGS_CONTAINER_H__

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

class CSymTorrentProxySettingsContainer: public CAknSettingItemList 
{
    public:
        /**
        * EPOC default constructor.
        * @param aAppUi SymTorrentAppUi.
        */
    	CSymTorrentProxySettingsContainer(CSymTorrentAppUi* aAppUi, CSymTorrentSettingsView* aView);
    	
        /**
        * EPOC default constructor.
        */
    	void ConstructL(const TRect& aRect);
    	
    	~CSymTorrentProxySettingsContainer();
    	
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
    
    	CSymTorrentAppUi*			iAppUi;
		
		TInetAddr					iProxyAddress;
		TInt						iProxyConnectionPort;
		TInt						iProxyServicePort;
		TInt						iIncomingConnections;	
		
		CSymTorrentSettingsView*	iView;
};

#endif

// End of File
