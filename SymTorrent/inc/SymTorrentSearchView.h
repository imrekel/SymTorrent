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
*  Name     : CSymTorrentTorrentSearchView from SymTorrentSearchView.h
*  Part of  : SymTorrent
*  Created  : 06.02.2007 by Imre Kelényi
* ============================================================================
*/

#ifndef SYMTORRENTSEARCHVIEW_H
#define SYMTORRENTSEARCHVIEW_H

// INCLUDES
#include <aknview.h>
#include "SymTorrentAppUi.h"

// FORWARD DECLARATIONS
class CSymTorrentSearchContainer;

// CLASS DECLARATION

/**
 * CSymTorrentSearchView view class. 
 */
class CSymTorrentSearchView : public CAknView
{
public: // Constructors and destructor

    void ConstructL(CSTTorrentManager* aTorrentMgr, CSymTorrentAppUi* aAppUi);

    ~CSymTorrentSearchView();

public: // Functions from base classes
                    
    TUid Id() const;

    void HandleCommandL(TInt aCommand);
    
    void HandleViewRectChange();

private:

    void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
        const TDesC8& aCustomMessage);

    void DoDeactivate();
    
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

private: 

    CSymTorrentSearchContainer*		iContainer;
    
	CSymTorrentAppUi*					iAppUi;
	
    CSTTorrentManager*					iTorrentMgr;
};

#endif

// End of File
