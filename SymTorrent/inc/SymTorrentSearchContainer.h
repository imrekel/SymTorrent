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
 *  Name     : CSymTorrentSearchContainer from SymTorrentSearchContainer.h
 *  Part of  : SymTorrent
 *  Created  : 06.02.2007 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENTSEARCHCONTAINER_H
#define SYMTORRENTSEARCHCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <aknlists.h> 
#include "SymTorrentAppUi.h"
#include "STTorrentManager.h"
#include "STDefs.h"
   
// FORWARD DECLARATIONS
class MAknsControlContext; // for skins support

// CLASS DECLARATION

/**
 *  CSymTorrentSearchContainer  container control class. 
 */
class CSymTorrentSearchContainer : public CCoeControl, public MCoeControlObserver
{
public: // Constructors and destructor
    
    void ConstructL(const TRect& aRect, CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr);

    ~CSymTorrentSearchContainer();

public: // New functions
	
	inline CAknDoubleGraphicStyleListBox* ListBox();
	
private: // Functions from base classes
   
    void SizeChanged();

    TInt CountComponentControls() const;

    CCoeControl* ComponentControl(TInt aIndex) const;

    void Draw(const TRect& aRect) const;

	TTypeUid::Ptr MopSupplyObject(TTypeUid aId);
   	
    void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
    
	TKeyResponse OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType);
		
private:
   
    CAknDoubleGraphicStyleListBox*			iListBox;
    
	CSymTorrentAppUi*						iAppUi;
		
    CSTTorrentManager*						iTorrentMgr;
};

inline CAknDoubleGraphicStyleListBox* CSymTorrentSearchContainer::ListBox() {
	return iListBox;
}

#endif

// End of File
