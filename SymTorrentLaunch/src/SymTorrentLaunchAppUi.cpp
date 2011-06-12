/*
============================================================================
 Name        : CSymTorrentLaunchAppUi from SymTorrentLaunchAppui.cpp
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : CSymTorrentLaunchAppUi implementation
============================================================================
*/

// INCLUDE FILES
#include "SymTorrentLaunchAppui.h"
#include "SymTorrentLaunchDocument.h"
#include <SymTorrentLaunch.rsg>
#include "SymTorrentLaunch.hrh"
#include <akndoc.h>

#include <utf.h>
#include <APGCLI.H>

#include <avkon.hrh>

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CSymTorrentLaunchAppUi::ConstructL()
// 
// ----------------------------------------------------------
//
void CSymTorrentLaunchAppUi::ConstructL()
{
    BaseConstructL();
}

// ----------------------------------------------------
// CSymTorrentLaunchAppUi::~CSymTorrentLaunchAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CSymTorrentLaunchAppUi::~CSymTorrentLaunchAppUi()
{
}

// ----------------------------------------------------
// CSymTorrentLaunchAppUi::HandleCommandL(TInt aCommand)
// takes care of command handling
// ----------------------------------------------------
//
void CSymTorrentLaunchAppUi::HandleCommandL(TInt aCommand)
{
    switch ( aCommand )
        {
        case EEikCmdExit:
            {
            Exit();
            break;
            }
        default:
            break;      
        }
}

// This function calls, when a torrent file is select in the y-browser, ... (other applications)
TBool CSymTorrentLaunchAppUi::ProcessCommandParametersL(TApaCommand aCommand,TFileName& aDocumentName,const TDesC8& aTail)
{
	if (aDocumentName.Length() > 0)
	{			
		static_cast<CSymTorrentLaunchDocument*>(Document())->HandleRecognizeL(aDocumentName);
		return ETrue;
	}
	else
		return CAknAppUi::ProcessCommandParametersL(aCommand, aDocumentName, aTail);
}
