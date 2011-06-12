/*
============================================================================
 Name        : CSymTorrentLaunchDocument from SymTorrentLaunchDocument.h
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Declares document for application.
============================================================================
*/

#ifndef SYMTORRENTLAUNCHDOCUMENT_H
#define SYMTORRENTLAUNCHDOCUMENT_H

// INCLUDES
#include <akndoc.h>
   
// CONSTANTS

// FORWARD DECLARATIONS
class  CEikAppUi;

// CLASS DECLARATION

/**
*  CSymTorrentLaunchDocument application class.
*/
class CSymTorrentLaunchDocument : public CAknDocument
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CSymTorrentLaunchDocument* NewL(CEikApplication& aApp);

        /**
        * Destructor.
        */
        virtual ~CSymTorrentLaunchDocument();

    public: // New functions
		// Switchs to SymTorrent
		void HandleRecognizeL(TFileName aFilename);
    public: // Functions from base classes
    protected:  // New functions

    protected:  // Functions from base classes
		// This function calls, when a torrent file is select in the filebrowser, ...
		void OpenFileL(CFileStore*& aFileStore, RFile& aFile);
    private:

        /**
        * EPOC default constructor.
        */
        CSymTorrentLaunchDocument(CEikApplication& aApp);
        
        void ConstructL();
        
    private:

        /**
        * From CEikDocument, create CSymTorrentLaunchAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();        
    };

#endif



