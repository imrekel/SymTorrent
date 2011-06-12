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
*  Name     : CSymTorrentIAPSelectView from SymTorrentIAPSelectView.h
*  Part of  : SymTorrent
*  Created  : 10.04.2006 by Imre Kelényi
* ============================================================================
*/

#ifndef SYMTORRENT_IAPSELECTVIEW_H__
#define SYMTORRENT_IAPSELECTVIEW_H__

// INCLUDES
#include <aknview.h>
#include "STDefs.h"

// FORWARD DECLARATIONS
class CSymTorrentIAPSelectContainer;

// CONSTS
const TUid KIAPSelectViewId = { 10 };

/**
 *  CSymTorrentIAPSelectView
 */
class CSymTorrentIAPSelectView : public CAknView
{
public: // Constructors and destructor

    void ConstructL();

    ~CSymTorrentIAPSelectView();

public: // Functions from base classes
    
    TUid Id() const;

    void HandleCommandL(TInt aCommand);

    void HandleClientRectChange();

private:

    void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
        const TDesC8& aCustomMessage);

    void DoDeactivate();
    
    void GetGPRSIAPsFromIAPTableL();

private:

    CSymTorrentIAPSelectContainer* iContainer;
    
    RArray<TUint32> iIAPIDs;
};


#endif

// End of File
 