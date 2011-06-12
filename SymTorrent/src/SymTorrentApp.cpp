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
 
/* ============================================================================
*  Name     : CSymTorrentApp from SymTorrentApp.cpp
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#ifdef EKA2
#include <eikstart.h>
#endif

#include    "SymTorrentApp.h"
#include    "SymTorrentDocument.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CSymTorrentApp::AppDllUid() const
    {
    return KUidSymTorrent;
    }

   
// ---------------------------------------------------------
// CSymTorrentApp::CreateDocumentL()
// Creates CSymTorrentDocument object
// ---------------------------------------------------------
//
CApaDocument* CSymTorrentApp::CreateDocumentL()
    {
    return CSymTorrentDocument::NewL( *this );
    }

// ================= OTHER EXPORTED FUNCTIONS ==============
//
// ---------------------------------------------------------
// NewApplication() 
// Constructs CSymTorrentApp
// Returns: created application object
// ---------------------------------------------------------
//
EXPORT_C CApaApplication* NewApplication()
    {
    return new CSymTorrentApp;
    }

#if defined(EKA2)

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }

#else

// ---------------------------------------------------------
// E32Dll(TDllReason) 
// Entry point function for EPOC Apps
// Returns: KErrNone: No error
// ---------------------------------------------------------
//
GLDEF_C TInt E32Dll( TDllReason )
    {
    return KErrNone;
    }

#endif //#if defined(EKA2)

// End of File  

