/*
============================================================================
 Name        : CSymTorrentLaunchAppUi from SymTorrentLaunchAppui.h
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Declares UI class for application.
============================================================================
*/

#ifndef SYMTORRENTLAUNCHAPPUI_H
#define SYMTORRENTLAUNCHAPPUI_H

// INCLUDES
#include <aknviewappui.h>

// CLASS DECLARATION

/**
* Application UI class.
* Provides support for the following features:
* - EIKON control architecture
* - view architecture
* - status pane
* 
*/
class CSymTorrentLaunchAppUi : public CAknViewAppUi
    {
    public: // // Constructors and destructor

        /**
        * EPOC default constructor.
        */      
        void ConstructL();

        /**
        * Destructor.
        */      
        ~CSymTorrentLaunchAppUi();
        
    public: // New functions

    public: // Functions from base classes

    private:

    private:
        /**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
        */
        void HandleCommandL(TInt aCommand);

		// This function calls, when a torrent file is select in the y-browser, ... (other applications)
		TBool ProcessCommandParametersL(TApaCommand aCommand,TFileName& aDocumentName,const TDesC8& aTail);

    private: //Data
    };

#endif


