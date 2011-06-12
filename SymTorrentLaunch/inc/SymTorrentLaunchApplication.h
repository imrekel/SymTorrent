/*
============================================================================
 Name        : SymTorrentLaunchApplication.h
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Main application class
============================================================================
*/

#ifndef __SYMTORRENTLAUNCHAPPLICATION_H__
#define __SYMTORRENTLAUNCHAPPLICATION_H__

// INCLUDES
#include <aknapp.h>

// CLASS DECLARATION

/**
* CSymTorrentLaunchApplication application class.
* Provides factory to create concrete document object.
* An instance of CSymTorrentLaunchApplication is the application part of the
* AVKON application framework for the SymTorrentLaunch example application.
*/
class CSymTorrentLaunchApplication : public CAknApplication
    {
    public: // Functions from base classes

        /**
        * From CApaApplication, AppDllUid.
        * @return Application's UID (KUidSymTorrentLaunchApp).
        */
        TUid AppDllUid() const;

    protected: // Functions from base classes

        /**
        * From CApaApplication, CreateDocumentL.
        * Creates CSymTorrentLaunchDocument document object. The returned
        * pointer in not owned by the CSymTorrentLaunchApplication object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
    };

#endif // __SYMTORRENTLAUNCHAPPLICATION_H__

// End of File
