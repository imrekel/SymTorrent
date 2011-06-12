/*
============================================================================
 Name        : SymTorrentLaunchApplication.cpp
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Main application class
============================================================================
*/

// INCLUDE FILES
#include "SymTorrentLaunchDocument.h"
#include "SymTorrentLaunchApplication.h"

// ============================ MEMBER FUNCTIONS ===============================

// UID for the application;
// this should correspond to the uid defined in the mmp file
const TUid KUidSymTorrentLaunchApp = { 0xA0001755 };

// -----------------------------------------------------------------------------
// CSymTorrentLaunchApplication::CreateDocumentL()
// Creates CApaDocument object
// -----------------------------------------------------------------------------
//
CApaDocument* CSymTorrentLaunchApplication::CreateDocumentL()
    {
    // Create an SymTorrentLaunch document, and return a pointer to it
    return (static_cast<CApaDocument*>
                    ( CSymTorrentLaunchDocument::NewL( *this ) ) );
    }

// -----------------------------------------------------------------------------
// CSymTorrentLaunchApplication::AppDllUid()
// Returns application UID
// -----------------------------------------------------------------------------
//
TUid CSymTorrentLaunchApplication::AppDllUid() const
    {
    // Return the UID for the SymTorrentLaunch application
    return KUidSymTorrentLaunchApp;
    }

// End of File
