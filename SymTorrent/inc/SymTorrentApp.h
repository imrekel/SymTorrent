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
*  Name     : CSymTorrentApp from SymTorrentApp.h
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
*  Description:
*     Declares main application class.
*  Version  :
*  Copyright: 2006
* ============================================================================
*/

#ifndef SYMTORRENTAPP_H
#define SYMTORRENTAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidSymTorrent = { 0xA0001751 };

// CLASS DECLARATION

/**
* CSymTorrentApp application class.
* Provides factory to create concrete document object.
* 
*/
class CSymTorrentApp : public CAknApplication
    {
    
    public: // Functions from base classes
    private:

        /**
        * From CApaApplication, creates CSymTorrentDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID (KUidSymTorrent).
        * @return The value of KUidSymTorrent.
        */
        TUid AppDllUid() const;
    };

#endif

// End of File

