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
*  Name     : CSymTorrentSettingsView from SymTorrentSettingsView.h
*  Part of  : SymTorrent
*  Created  : 13.03.2006 by Péter Ekler
*  Description:
*     Declares the settings view for application.
*  Version  :
*  Copyright: 2006
* ============================================================================
*/

#ifndef SYMTORRENTSETTINGSVIEW_H
#define SYMTORRENTSETTINGSVIEW_H

// INCLUDES
#include <aknview.h>
#include "STDefs.h"

enum TSymTorrentSettingType
{
	ESettingGeneral = 1,
	ESettingProxy,
	ESettingTracker
};


// FORWARD DECLARATIONS
class CAknSettingItemList;
class CAknNavigationDecorator;
class CAknTabGroup;

// CLASS DECLARATION

/**
*  CSymTorrentSettingsView view class.
* 
*/
class CSymTorrentSettingsView : public CAknView
    {
    public: // Constructors and destructor
      
      	CSymTorrentSettingsView();      	 
      	 
        void ConstructL();
    
        ~CSymTorrentSettingsView();

    public: // Functions from base classes
        
        TUid Id() const;
       
        void HandleCommandL(TInt aCommand);
      
        void HandleViewRectChange();
        
        void SetActiveSettingPage(TSymTorrentSettingType aSettingPage);

    private:
       
        void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
            const TDesC8& aCustomMessage);
            
        void DoDeactivate();

    private: // Data

        CAknSettingItemList* iSettingsContainer;

		/**
		 * The ID of the previous view.
		 */
		TVwsViewId iPrevViewId;
		
		TSymTorrentSettingType iActiveSettingPage;
		
		CAknNavigationDecorator* 	iNaviTabDecorator;
		CAknTabGroup*				iNaviTabGroup;
    };

#endif

// End of File
