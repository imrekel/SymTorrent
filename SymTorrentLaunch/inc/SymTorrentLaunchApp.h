/*
============================================================================
 Name        : CSymTorrentLaunchApp from SymTorrentLaunchApp.h
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Declares main application class.
============================================================================
*/

#ifndef SYMTORRENTLAUNCHAPP_H
#define SYMTORRENTLAUNCHAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidSymTorrentLaunch = { 0xA0001755 };

// CLASS DECLARATION

/**
* CSymTorrentLaunchApp application class.
* Provides factory to create concrete document object.
* 
*/
class CSymTorrentLaunchApp : public CAknApplication
    {
    
    public: // Functions from base classes
    private:

        /**
        * From CApaApplication, creates CSymTorrentLaunchDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID (KUidSymTorrentLaunch).
        * @return The value of KUidSymTorrentLaunch.
        */
        TUid AppDllUid() const;
    };

#endif



