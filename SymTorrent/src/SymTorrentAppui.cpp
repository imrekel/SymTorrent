/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
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
 * along with SymTorrent; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/**
 * ============================================================================
 *  Name     : CSymTorrentAppUi from SymTorrentAppui.cpp
 *  Part of  : SymTorrent
 *  Created  : 31.01.2006 by Imre Kelényi
 *  Copyright: 2006
 * ============================================================================
 */

// INCLUDE FILES
#include <SymTorrent.rsg>
#include <avkon.hrh>
#include <barsread.h>
#include <akntabgrp.h> 
#include <StringLoader.h>
#include <apparc.h>
#include <apgcli.h>
#include <aknmessagequerydialog.h>
#include <utf.h>
#include <apgtask.h>
#include <caknfileselectiondialog.h>
#include <caknmemoryselectiondialog.h> 
#include <pathinfo.h> 
#include <aknnotewrappers.h>
#include <aknwaitdialog.h>
#include <aknsutils.h> 
#include "SymTorrentLog.h"
#include "SymTorrent.hrh"
#include "SymTorrentAppui.h"
#include "SymTorrentMainView.h"
#include "SymTorrentDetailsView.h"
#include "SymTorrentDownloadStateView.h"
#include "SymTorrentFilesView.h"
#include "SymTorrentSettingsView.h"
#include "SymTorrentStatusView.h"
#include "STTorrentManagerSingleton.h"
#include "STPreferences.h"
#include "STTorrent.h"
#include "AccessPointSettingItem.h"
#include "STGlobal.h"

#ifdef USE_DHT
#include "BitTorrentDHT.h"
#endif

#ifdef TRACKER_BUILD
#include "SymTorrentTrackerManager.h"
#endif

// Constants
_LIT(KLitSymTorrentAbout, "SymTorrent ");
_LIT(KLitBuiltOn, "Built on ");
_LIT(KSecInfo, "\nhttp://symtorrent.aut.bme.hu\nAuthors:\nImre Kelenyi\nPeter Ekler\n\nDeveloped by AMORG (Applied Mobile Research Group) - Department of Automation and Applied Informatics - Budapest University Of Technology and Economics\nVisit http://symtorrent.aut.bme.hu for more information.\nThis is a free software. (license: GNU GPL)\n");

#ifdef EKA2
_LIT(KSTPreferencesFile, "c:\\private\\A0001751\\symtorrent.cfg");
#else
_LIT(KSTPreferencesFile, "c:\\system\\apps\\SymTorrent\\symtorrent.cfg");
#endif

const TInt KCloseTimeout = 1;

// ================= MEMBER FUNCTIONS =======================

void CSymTorrentAppUi::ConstructL()
{
    BaseConstructL( EAknEnableSkin );
    
    // adds the resources needed by the tracker's UI
	#ifdef TRACKER_BUILD
	#ifdef __WINS__
    	iSymTrackerResourceOffset = CCoeEnv::Static()->AddResourceFileL(_L("z:\\resource\\apps\\SymTracker.rsc"));
	#else
    	iSymTrackerResourceOffset = CCoeEnv::Static()->AddResourceFileL(_L("c:\\resource\\apps\\SymTracker.rsc"));
	#endif    	
	#endif
    	
	// queries the default text color from the UI theme
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	AknsUtils::GetCachedColor(skin, iTextColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6);

    iTorrentMgr = TORRENTMGR;
    iTorrentMgr->Preferences()->SetPreferencesFileL(KSTPreferencesFile);
    iTorrentMgr->Preferences()->LoadSettingsL();
    
    iTorrentMgr->NetworkManager()->SetAccessPointSupplier(this);
    iTorrentMgr->NetworkManager()->AddObserverL(this);
    
    iTorrentMgr->SetEngineEventObserver(this);
    
	#ifdef LOG_TO_FILE
    iLog = LOG;
	#endif
        
	CEikStatusPane* statusPane = StatusPane();
	
	iTitlePane = (CAknTitlePane*)statusPane->ControlL(
		TUid::Uid( EEikStatusPaneUidTitle ) );
		
	TUid paneUid;
	paneUid.iUid = EEikStatusPaneUidNavi;
	if (statusPane->PaneCapabilities(paneUid).IsPresent() && 
		statusPane->PaneCapabilities(paneUid).IsAppOwned())
	{
		iNaviPane = (CAknNavigationControlContainer*) statusPane->ControlL(paneUid);
		iDecorator = iNaviPane->CreateTabGroupL();
		iTabGroup = (CAknTabGroup*) iDecorator->DecoratedControl();
		
		iTabGroup->SetTabFixedWidthL(KTabWidthWithThreeTabs);
		
		iTabGroup->SetObserver( this );

		HBufC* text1 = StringLoader::LoadLC(R_DOWNLOADSTATEVIEW_TITLE);
		iTabGroup->AddTabL(ESymTorrentDownloadStateViewTab, *text1);
		CleanupStack::PopAndDestroy();				

		HBufC* text3 = StringLoader::LoadLC(R_FILESVIEW_TITLE);
		iTabGroup->AddTabL(ESymTorrentFilesViewTab, *text3);
		CleanupStack::PopAndDestroy();
		
		HBufC* text2 = StringLoader::LoadLC(R_DETAILSVIEW_TITLE);
		iTabGroup->AddTabL(ESymTorrentDetailsViewTab, *text2);
		CleanupStack::PopAndDestroy();
		
		//iTabGroup->AddTabL(ESymTorrentDetailsViewTab + 1, _L("Trackers"));

		iTabGroup->SetActiveTabByIndex(0);	
		
		iMainNaviDecorator = iNaviPane->CreateTabGroupL();
		iMainTabGroup = (CAknTabGroup*) iMainNaviDecorator->DecoratedControl();
		iMainTabGroup->SetTabFixedWidthL(KTabWidthWithTwoTabs);
		iMainTabGroup->AddTabL(ESymTorrentMainViewTab, _L("Torrents"));
		iMainTabGroup->AddTabL(ESymTorrentStatusViewTab, _L("Status"));
		
		iNaviPane->PushL(static_cast<CAknNavigationDecorator &>(*iMainNaviDecorator));
		iMainTabGroup->SetActiveTabByIndex(0);
		
		iMainNaviDecoratorActive = ETrue;
		
	}

	isEnabledNaviViewChange = EFalse;
	
    CSymTorrentMainView* mainView = new (ELeave) CSymTorrentMainView;
    CleanupStack::PushL( mainView );
    mainView->ConstructL(iTorrentMgr,this);
    AddViewL( mainView );   // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // mainView

    CSymTorrentDownloadStateView* DownloadStateView = new (ELeave) CSymTorrentDownloadStateView;
    CleanupStack::PushL( DownloadStateView );
    DownloadStateView->ConstructL(iTorrentMgr,this);
    AddViewL( DownloadStateView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // DownloadStateView

    CSymTorrentDetailsView* DetailsView = new (ELeave) CSymTorrentDetailsView;
    CleanupStack::PushL( DetailsView );
    DetailsView->ConstructL(iTorrentMgr,this);
    AddViewL( DetailsView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // DetailsView

    CSymTorrentFilesView* FilesView = new (ELeave) CSymTorrentFilesView;
    CleanupStack::PushL( FilesView );
    FilesView->ConstructL(iTorrentMgr, this);
    AddViewL( FilesView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // FilesView

    CSymTorrentSettingsView* SettingsView = new (ELeave) CSymTorrentSettingsView;
    CleanupStack::PushL( SettingsView );
    SettingsView->ConstructL();
    AddViewL( SettingsView );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // SettingsView
    
    CSymTorrentStatusView* statusView = new (ELeave) CSymTorrentStatusView;
    CleanupStack::PushL( statusView );
    statusView->ConstructL(iTorrentMgr, this);
    AddViewL( statusView );
    CleanupStack::Pop();    // statusView

	#ifdef TRACKER_BUILD
		iTrackerManager = new (ELeave) CSymTorrentTrackerManager(this, iTorrentMgr, this);
		iTrackerManager->ConstructL();
	#endif

	SetDefaultViewL(*mainView);
       
	iFileSelectionType = EStartC;
	
	// Checking the notify File
	RFs fileServer;
	RFile tempFile;
	User::LeaveIfError(fileServer.Connect());	
	fileServer.MkDirAll(KNotifyFile);
	if (tempFile.Replace(fileServer,KNotifyFile,EFileWrite)==KErrNone)
		tempFile.Close();
	fileServer.Close();	
	
	iNotifyFileChange = new (ELeave) CNotifyFileChange();
 	iNotifyFileChange->ConstructL(this);
 	
 	iTorrentMgr->LoadSavedTorrentsL();		
	
	iStartupTimer = CPeriodic::NewL(0); // neutral priority
	// the timer ticks in every second
	iStartupTimer->Start(100, 100,
		TCallBack(StaticOnStartupTimerL, this));	
}


CSymTorrentAppUi::~CSymTorrentAppUi()
{
	if (iCloseTimer)
	{
		iCloseTimer->Cancel();
		delete iCloseTimer;
	}
	
	delete iStartupTimer;

	#ifdef TRACKER_BUILD	
		delete iTrackerManager;
	#endif
	
	RemoveNaviPaneText();
	
	delete iDecorator;	
	delete iMainNaviDecorator;
	
	delete iNotifyFileChange;
	
#ifdef TRACKER_BUILD
	CCoeEnv::Static()->DeleteResourceFile(iSymTrackerResourceOffset);
#endif

#ifdef EKA2	
	delete iDocHandler;
#endif
}

CAknNavigationControlContainer* CSymTorrentAppUi::NaviPane()
{
	TUid paneUid;
	paneUid.iUid = EEikStatusPaneUidNavi;
	
	return (CAknNavigationControlContainer*)StatusPane()->ControlL(paneUid);
}

void CSymTorrentAppUi::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
{
	if ((aResourceId == R_SYMTORRENT_MAINVIEW_MENU) || (aResourceId == R_SYMTORRENT_STATUSVIEW_MENU))
	{
	/*	if (PREFERENCES->IncomingConnectionsMode() != EEnabledWithProxy)
		{*/
			aMenuPane->SetItemDimmed(ESymTorrentCmdConnectProxy, ETrue);
			aMenuPane->SetItemDimmed(ESymTorrentCmdDisconnectProxy, ETrue);
		/*}
		else
		{
			if (TORRENTMGR->NetworkManager()->ProxyConnectorState() == EConOffline)
			{
				aMenuPane->SetItemDimmed(ESymTorrentCmdConnectProxy, EFalse);
				aMenuPane->SetItemDimmed(ESymTorrentCmdDisconnectProxy, ETrue);
			}
			else
			{
				aMenuPane->SetItemDimmed(ESymTorrentCmdConnectProxy, ETrue);
				aMenuPane->SetItemDimmed(ESymTorrentCmdDisconnectProxy, EFalse);
			}
		}*/
	}
}

// ----------------------------------------------------
// CSymTorrentAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// takes care of key event handling
// ----------------------------------------------------
//
TKeyResponse CSymTorrentAppUi::HandleKeyEventL(
    const TKeyEvent& aKeyEvent,TEventCode aType)
{
	if ( iTabGroup == NULL )
    {
        return EKeyWasNotConsumed;
    }

    if ( aKeyEvent.iCode == EKeyLeftArrow || aKeyEvent.iCode == EKeyRightArrow )
    {
		if ( isEnabledNaviViewChange )
			return iTabGroup->OfferKeyEventL( aKeyEvent, aType );
		else return EKeyWasNotConsumed;
    }
    else
    {
        return EKeyWasNotConsumed;
    }
}

void CSymTorrentAppUi::ExitSymTorrentL()
{
	#ifdef TRACKER_BUILD	
		iTrackerManager->SaveTrackerDatasToFile();
		
		delete iTrackerManager;	
		iTrackerManager = 0;			
	#endif
				
	CSTTorrentManager* torrentMgr = TORRENTMGR;
	if (iDoorObserver)
		iDoorObserver->NotifyExit(MApaEmbeddedDocObserver::ENoChanges);
		
	if (!iSavedBeforeExit)
	{
		torrentMgr->SaveTorrentsStateL();
		PREFERENCES->SaveSettingsL();
		
	#ifdef USE_DHT
		torrentMgr->DHT()->SaveConfig();
	#endif
		iSavedBeforeExit = ETrue;
	}
	
	if ((torrentMgr->TorrentCount() > 0) && (iWaitDialog == 0))
	{	
		iWaitDialog = new (ELeave) CAknWaitDialog( 
			(REINTERPRET_CAST(CEikDialog**,&iWaitDialog))); 
		iWaitDialog->SetCallback(this);
		iWaitDialog->ExecuteLD(R_SYMTORRENT_WAIT_DIALOG);	
	}
	
	torrentMgr->CloseAllTorrentsL(this);
	
//	Exit();
}

void CSymTorrentAppUi::SetNaviPaneTextL(const TDesC& aText)
{
	if (iLabelNaviDecorator)
		RemoveNaviPaneText();
	
	iLabelNaviDecorator = iNaviPane->CreateNavigationLabelL( aText );
	iNaviPane->PushL( *iLabelNaviDecorator );
}

void CSymTorrentAppUi::RemoveNaviPaneText()
{
	if (iLabelNaviDecorator)
	{
		iNaviPane->Pop(iLabelNaviDecorator);
		//NaviPane()->PushDefaultL();
		delete iLabelNaviDecorator;
		iLabelNaviDecorator = 0;		
	}
}

// ----------------------------------------------------
// CSymTorrentAppUi::HandleCommandL(TInt aCommand)
// takes care of command handling
// ----------------------------------------------------
//
void CSymTorrentAppUi::HandleCommandL(TInt aCommand)
{
    switch ( aCommand )
    {
    	case EEikCmdExit:
    		ExitSymTorrentL();
    	break;
    	
    	case EAknSoftkeyBack:
		case EAknSoftkeyExit:
		{
        	if ((TORRENTMGR->TorrentCount() == 0) || 
        		(iEikonEnv->QueryWinL(_L("Quit SymTorrent?"), _L(""))))
        	{
        		ExitSymTorrentL();
        	}        		
        }			
        break;
 
 	#ifdef TRACKER_BUILD       
    	case ETrackerCmdStartTracker:
    	{
    		TrackerCmdStartTracker();
    	}
    	break;
   #endif
        
        case ESymTorrentCmdChooseTorrentFile:
        {
			TBuf<256> filename;

			if (FileSelectQueryL(filename))
			{
				ActivateMainViewL();
				InsertFileL(filename);				
			}
	        break;
        }
        
    	case ESymTorrentCmdSettings:
		{
			ActivateSettingsViewL();
			break;
		}
		
		case ESymTorrentCmdStatus:
		{
			ActivateStatusViewL();
			break;
		}

		#ifdef TRACKER_BUILD	
	    	case ESymTorrentCmdTracker:
			{
				ActivateTrackerTorrentListViewL();
				break;
			}
		#endif

		case ESymTorrentCmdAbout:
		{
			TBuf<30> time;
			TBuf<50> date;
			date.Copy(_L8(__DATE__));
			HBufC* info = HBufC::NewLC(TPtrC(KLitSymTorrentAbout).Length() +
				TPtrC(KLitBuiltOn).Length() + date.Length() + TPtrC(KSecInfo).Length() + 128);
			TPtr des = info->Des();
			
			des.Copy(KLitSymTorrentAbout);
			des.Append(SYMTORRENT_VERSION_LIT);
			
			#ifdef TRACKER_BUILD
				des.Append(_L("\nwith Tracker"));
			#endif
			
			#ifdef LOG_TO_FILE
				des.Append(_L("\nLOGGING TO FILE"));
			#endif		
			des.Append(KSecInfo);			
			des.Append(KLitBuiltOn);
			des.Append(date);

			CAknMessageQueryDialog* dlg = new (ELeave)CAknMessageQueryDialog();
			dlg->PrepareLC( R_DATA_DIALOG );
			dlg->SetMessageTextL(des);
			dlg->QueryHeading()->SetTextL(_L("About SymTorrent"));
			dlg->RunLD();

			CleanupStack::PopAndDestroy(); //info						

			break;
		}
		
    	case ESymTorrentCmdHide:
    	{
    		TApaTask task(CEikonEnv::Static()->WsSession());
			task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
			task.SendToBackground();
    	}
    	break;
    	
        case ESymTorrentCmdMainPage:
        {
            ActivateLocalViewL(TUid::Uid(ESymTorrentMainView));
			DisableNaviViewChange();
            break;
        }
        
    	case ESymTorrentCmdConnectProxy:
    	{
    		//TORRENTMGR->NetworkManager()->ConnectProxyL();
    		break;
    	}
    	
    	case ESymTorrentCmdDisconnectProxy:
    	{
    		//TORRENTMGR->NetworkManager()->DisconnectProxy();
    		break;
    	}
        
        default:
            break;      
    }
}


// ----------------------------------------------------
// CSymTorrentAppUi::TabChangedL(TInt aIndex)
// This method gets called when CAknTabGroup active 
// tab has changed.
// ----------------------------------------------------
//
void CSymTorrentAppUi::TabChangedL(TInt aIndex)
{
	switch (aIndex)
	{
		case 0:
			ActivateLocalViewL(TUid::Uid(ESymTorrentDownloadStateView));
		break;
		
		case 1:
			ActivateLocalViewL(TUid::Uid(ESymTorrentFilesView));
		break;
		
		case 2:
			ActivateLocalViewL(TUid::Uid(ESymTorrentDetailsView));
		break;
		
		default:;
		
	}
}


// ----------------------------------------------------
// CSymTorrentAppUi::EnableNaviViewChange()
// Enables the navi view change
// ----------------------------------------------------
//
void CSymTorrentAppUi::EnableNaviViewChange()
{
	if (!isEnabledNaviViewChange)
	{
		iNaviPane->PushL(static_cast<CAknNavigationDecorator &>(*iDecorator));
		isEnabledNaviViewChange=ETrue;
	}
}

// ----------------------------------------------------
// CSymTorrentAppUi::DisableNaviViewChange()
// Disables the navi view change
// ----------------------------------------------------
//
void CSymTorrentAppUi::DisableNaviViewChange()
{
	if (isEnabledNaviViewChange)
	{
		iNaviPane->Pop();
		isEnabledNaviViewChange = EFalse;
	}	
}

// ----------------------------------------------------
// CSymTorrentAppUi::ActivateDownloadStateViewL()
// Shows the Torrent DownloadState View
// ----------------------------------------------------
//
void CSymTorrentAppUi::ActivateDownloadStateViewL()
{
	DeactivateMainTabDecorator();
	ActivateLocalViewL(TUid::Uid(ESymTorrentDownloadStateView));
	EnableNaviViewChange();
	iTabGroup->SetActiveTabByIndex(0);
}

// ----------------------------------------------------
// CSymTorrentAppUi::ActivateTorrentDetailView()
// Shows the TorrentDetailView
// ----------------------------------------------------
//
void CSymTorrentAppUi::ActivateTorrentDetailViewL()
{
	DeactivateMainTabDecorator();
	ActivateLocalViewL(TUid::Uid(ESymTorrentDetailsView));
	EnableNaviViewChange();
	iTabGroup->SetActiveTabByIndex(2);
}

// ----------------------------------------------------
// CSymTorrentAppUi::ActivateFilesViewL()
// Shows the Torrent FilesView
// ----------------------------------------------------
//
void CSymTorrentAppUi::ActivateFilesViewL()
{
	DeactivateMainTabDecorator();
	ActivateLocalViewL(TUid::Uid(ESymTorrentFilesView));
	EnableNaviViewChange();
	iTabGroup->SetActiveTabByIndex(1);
}

// ----------------------------------------------------
// CSymTorrentAppUi::ActivateMainViewL()
// Shows the MainView
// ----------------------------------------------------
//
void CSymTorrentAppUi::ActivateMainViewL()
{
#ifdef TRACKER_BUILD
	iTrackerManager->SetInTracker(EFalse);
#endif
    ActivateLocalViewL(TUid::Uid(ESymTorrentMainView));
	DisableNaviViewChange();
	
	ActivateMainTabDecoratorL();
	iMainTabGroup->SetActiveTabById(ESymTorrentMainViewTab);	
}

void CSymTorrentAppUi::ActivateMainTabDecoratorL()
{
	if (!iMainNaviDecoratorActive)
	{
		iNaviPane->PushL(static_cast<CAknNavigationDecorator &>(*iMainNaviDecorator));
		iMainNaviDecoratorActive = ETrue;
	}
}

void CSymTorrentAppUi::DeactivateMainTabDecorator()
{
	if (iMainNaviDecoratorActive)
	{
		iNaviPane->Pop();
		iMainNaviDecoratorActive = EFalse;
	}
}

void CSymTorrentAppUi::ActivateViewL(TSymTorrentViewId aViewId)
{
	switch (aViewId)
	{
		case ESymTorrentMainView:
			ActivateMainViewL();
			break;
			
		case ESymTorrentDownloadStateView:
			ActivateDownloadStateViewL();
			break;
			
		case ESymTorrentDetailsView:
			ActivateTorrentDetailViewL();
			break;
			
		case ESymTorrentFilesView:	
			ActivateFilesViewL();
			break;
			
		case ESymTorrentSettingsView:
			ActivateSettingsViewL();
			break;
			
		case ESymTorrentStatusView:
			ActivateStatusViewL();
			break;
			
		/*case ESymTorrentTrackerTorrentListView:
		case ESymTorrentMakerFileListView:
		case ESymTorrentMakerDetailsView:*/
		default:
			ActivateLocalViewL(TUid::Uid(aViewId));		
	}	
}

// ----------------------------------------------------
// CSymTorrentAppUi::ActivateSettingsViewL()
// Shows the SettingsView
// ----------------------------------------------------
//
void CSymTorrentAppUi::ActivateSettingsViewL()
{
	DeactivateMainTabDecorator();
    ActivateLocalViewL(TUid::Uid(ESymTorrentSettingsView));
}

// ----------------------------------------------------
// CSymTorrentAppUi::SetSelectedTorrentIndex()
// Sets the selected torrent index
// ----------------------------------------------------
//
void CSymTorrentAppUi::SetSelectedTorrentIndex(TInt aIndex)
{
	iSelectedTorrentIndex=aIndex;
}
// ----------------------------------------------------
// CSymTorrentAppUi::SelectedTorrentIndex()
// Returns the selected torrent index
// ----------------------------------------------------
//
TInt CSymTorrentAppUi::SelectedTorrentIndex()
{
	return iSelectedTorrentIndex;
}

TBool CSymTorrentAppUi::ProcessCommandParametersL(TApaCommand aCommand,TFileName& aDocumentName,const TDesC8& aTail)
{
	LWRITE(iLog, _L("ProcessCommandParametersL: "));
	LWRITELN(iLog, aDocumentName);
	LWRITE(iLog, _L(" tail: "));
	LWRITELN(iLog, aTail);
	
	if (aDocumentName.Length() > 0)
	{ 
		TInt res;
		if ((aDocumentName.Length() >= 9) && (aDocumentName.Right(9) == _L(".storrent")))
			res = iTorrentMgr->OpenSavedTorrentL(aDocumentName);
		else
			res = iTorrentMgr->OpenTorrentL(aDocumentName);
		
		if (res == KErrNone)
		{
			SetSelectedTorrentIndex(TORRENTMGR->TorrentCount() - 1);
			ActivateFilesViewL();
		}
		
		return ETrue;
	}
	
	return CEikAppUi::ProcessCommandParametersL(aCommand, aDocumentName, aTail);
}

TBool CSymTorrentAppUi::GetIapIdL(TInt32& aAccesPointId, TDes& aAccesPointName, TInt /*aNetConnIndex*/)
{
	LWRITELN(iLog, _L("[AppUi] Asking for access point"));
	TApaTask task(iEikonEnv->WsSession());
	task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
	task.BringToForeground();
	
	TInt32 iap = 0;
	TBuf<128> apName;
	
	CAccessPointSettingItem* iapSettingItem = 
		new (ELeave) CAccessPointSettingItem(1, iap, apName);
		
	CleanupStack::PushL(iapSettingItem);
	
	TBool changed = iapSettingItem->AskIapIdL(iap, aAccesPointName);
	
	CleanupStack::PopAndDestroy(); // iapSettingItem
	
	aAccesPointId = iap;
	return changed;
}

void CSymTorrentAppUi::ProcessMessageL(TUid aUid,const TDesC8& aParams)
{
	LWRITE(iLog, _L("ProcessMessageL: "));
	LWRITELN(iLog, aParams);
	if (aUid == TUid::Uid(KUidApaMessageSwitchOpenFileValue))
	{
		// the filename arrives in utf8 format, we have to convert it back to unicode
		HBufC* filename = HBufC::NewLC(aParams.Length());
		TPtr filenamePtr = filename->Des();

		if (CnvUtfConverter::ConvertToUnicodeFromUtf8(filenamePtr, aParams) == 0)
		{
			LWRITELN(iLog, *filename);
			
			TInt res;
			if ((filename->Length() >= 9) && (filename->Right(9) == _L(".storrent")))
				res = iTorrentMgr->OpenSavedTorrentL(*filename);
			else
				res = iTorrentMgr->OpenTorrentL(*filename);
			
			if (res == KErrNone)
			{
				SetSelectedTorrentIndex(TORRENTMGR->TorrentCount() - 1);
				ActivateFilesViewL();
			}
		}				

		CleanupStack::PopAndDestroy(); // filename
	}
	else
		CAknAppUi::ProcessMessageL(aUid, aParams);
}

void CSymTorrentAppUi::InsertFileL(const TDesC& aData)
{

	CSTTorrent* torrent = NULL;
	TInt value = iTorrentMgr->OpenTorrentL(aData, torrent);
	
	if (value != KErrNone)
	{
		TBuf<100> MessageString;
		switch ( value )
		{			
			case KErrParsingFailed:
			{
				MessageString.Append(_L("Failed to parse torrent! (wrong format)"));
				break;
			}
			case KErrGeneral:
			default:
			{
				MessageString.Append(_L("Failed to load torrent!"));
				break;
			}
		}

		CAknWarningNote* note = new(ELeave) CAknWarningNote;
		note->ExecuteLD(MessageString);
	}
	else
	{
		CheckHashL(torrent);
		
		TBool startTorrent = ETrue;
		if (!torrent->HasEnoughDiskSpaceToDownload())
			if (!iEikonEnv->QueryWinL(_L("There is not enough free disk space for full download. Do you want to continue?"), _L("")))
				startTorrent = EFalse;
		
		if (torrent->SkipTooLargeFiles() > 0)
		{
			CAknQueryDialog* dlg = CAknQueryDialog::NewL();
			dlg->PrepareLC(R_WARNING_QUERY);
			dlg->SetPromptL(_L("The torrent has at least one file that is larger than 4GB. This file cannot be downloaded due to the file system (FAT32) limitations."));
			dlg->RunLD();
		}
			
		if (startTorrent)
			torrent->StartL();
	}
}

TBool CSymTorrentAppUi::FileSelectQueryL(TFileName &aFileName)
{
	// Select memory
	CAknMemorySelectionDialog* memSelectionDialog = CAknMemorySelectionDialog::NewL
		(ECFDDialogTypeNormal, /*aShowUnavailableDrives*/EFalse);
	CleanupStack::PushL(memSelectionDialog);

	CAknMemorySelectionDialog::TMemory mem(CAknMemorySelectionDialog::EPhoneMemory);

	TInt ret = memSelectionDialog->ExecuteL(mem);
	CleanupStack::PopAndDestroy(memSelectionDialog);
	if (!ret) return EFalse;

	// get root on phone memory or card
	//TFileName pathname;
	CAknFileSelectionDialog* fileSelectionDialog;	
	if (mem == CAknMemorySelectionDialog::EMemoryCard)
	{		
		//pathname.Append(PathInfo::MemoryCardRootPath());
		//aFileName.Append(_L("E:\\"));
		fileSelectionDialog= CAknFileSelectionDialog::NewL(ECFDDialogTypeNormal, 
			R_TORRENTFILE_SELECTION_DIALOG_E );
	}
	else
	{		
		//pathname.Append(PathInfo::PhoneMemoryRootPath());
		//aFileName.Append(_L("C:\\"));
		fileSelectionDialog= CAknFileSelectionDialog::NewL(ECFDDialogTypeNormal, 
			R_TORRENTFILE_SELECTION_DIALOG_C );
	}	

	// launch file select dialog
	TBool result=fileSelectionDialog->ExecuteL(aFileName);
	delete fileSelectionDialog;
	return result;
}

void CSymTorrentAppUi::HandleResourceChangeL(TInt aType) 
{ 
    CAknAppUi::HandleResourceChangeL( aType ); 
    
	#ifdef EKA2
   	if ( aType == KAknsMessageSkinChange || aType == KEikDynamicLayoutVariantSwitch ) 
    { 
    	View(TUid::Uid(iCurrentViewId))->HandleViewRectChange();
    	View(TUid::Uid(iCurrentViewId))->Redraw();
    } 
	#endif 
} 

void CSymTorrentAppUi::ActivateStatusViewL()
{	
	ActivateLocalViewL(TUid::Uid(ESymTorrentStatusView));
	
	DisableNaviViewChange();
	ActivateMainTabDecoratorL();
	iMainTabGroup->SetActiveTabById(ESymTorrentStatusViewTab);
}

void CSymTorrentAppUi::TorrentsClosedL()
{
	if (iWaitDialog)
	{
		if ((iCloseTimer == 0) || (!iCloseTimer->IsActive()))
		{
			delete iCloseTimer;
			iCloseTimer = 0;
			
			iCloseTimeoutCounter = KCloseTimeout;
			iCloseTimer = CPeriodic::NewL(0); // neutral priority
			// the timer ticks in every second
	    	iCloseTimer->Start(1000000, 1000000,
				TCallBack(StaticOnCloseTimerL, this));	
		}
		
		iWaitDialog->ProcessFinishedL();
		
	
	}
	else		
		Exit();
}

void CSymTorrentAppUi::DialogDismissedL(TInt /*aButtonId*/)
{
	delete iWaitDialog;
	iWaitDialog = 0;
		
	Exit();
}

TInt CSymTorrentAppUi::StaticOnCloseTimerL(TAny* aObject)
{
	// cast, and call non-static function
	((CSymTorrentAppUi*)aObject)->OnCloseTimerL();
	return 1;
}
	
void CSymTorrentAppUi::OnCloseTimerL()
{
	iCloseTimer->Cancel();

	Exit();
}

TInt CSymTorrentAppUi::StaticOnStartupTimerL(TAny* aObject)
{
	// cast, and call non-static function
	((CSymTorrentAppUi*)aObject)->OnStartupTimerL();
	return 1;
}
	
void CSymTorrentAppUi::OnStartupTimerL()
{
	iStartupTimer->Cancel();
	delete iStartupTimer;
	iStartupTimer = NULL;
	
	if (iTorrentMgr->Preferences()->StartupHashCheck())
	{
		for (TInt i=0; i<iTorrentMgr->TorrentCount(); i++)
	 		CheckHashL(iTorrentMgr->Torrent(i));
	}	
}

void CSymTorrentAppUi::CheckHashL(CSTTorrent* aTorrent)
{	
	if (!iProgressDialog)
	{
		iProgressDialog = new (ELeave) CAknProgressDialog( 
			(REINTERPRET_CAST(CEikDialog**,&iProgressDialog)), ETrue); 
		//waitDialog->SetCallback(this);
		
		iProgressDialog->PrepareLC(R_HASH_CHECK_PROGRESS_NOTE);
		TBuf<100> dialogText;
		dialogText.Append(_L("Looking for already downloaded files of \""));
		//dialogText.Append(_L("Checking hash of \""));
		dialogText.Append(aTorrent->Name().Left(40));
		dialogText.Append(_L("\""));
		
		iProgressDialog->SetTextL(dialogText);
		
		aTorrent->CheckHashL(iProgressDialog);
		
		iProgressDialog->RunLD();
		
		iProgressDialog = NULL;		
	}
}

void CSymTorrentAppUi::HandleTorrentCriticalFaultL(CSTTorrent* aTorrent, TSTCriticalFaultType aFault)
{
	TBuf<200> msg;
	msg = _L("Torrent: ");
	msg.Append(aTorrent->Name().Left(35));
	if (aTorrent->Name().Length() > 35)
		msg.Append(_L("..."));
	msg.Append(_L("\n"));
	
	CAknQueryDialog* dlg = CAknQueryDialog::NewL();
	dlg->PrepareLC(R_WARNING_QUERY);
	
	switch (aFault)
	{
		case EWritingToDiskFailed:
			msg.Append(_L("Critical fault.\nWriting to disk failed. Disk might be full."));
		break;
		
		case ENoPeersLeft:
			msg.Append(_L("Critical fault.\nNo active peers or failed to connect tracker."));
		break;
		
		default:
			msg.Append(_L("Critical fault."));
		break;
	}
	
	dlg->SetPromptL(msg);
	dlg->RunLD();
}

void CSymTorrentAppUi::HandleNotifyMessageL()
{
	RFs fileServer;   
	User::LeaveIfError(fileServer.Connect());
	fileServer.Rename(KNotifyFileMod,KNotifyFile);

	TFindFile file_finder(fileServer);
	CDir* file_list; 
	TInt err = file_finder.FindWildByDir(_L("*.mnt"),KNotifyDir,file_list);

	LWRITELN(iLog, _L("Notify1"));
	if (err == KErrNone)
	{  
		LWRITELN(iLog, _L("Notify2"));
		for (TInt i=0; i<file_list->Count(); i++)
		{
			LWRITELN(iLog, _L("Notify3"));
			TParse fullentry;
			fullentry.Set((*file_list)[i].iName,& file_finder.File(),NULL);
			RFile tempFile;
			if (tempFile.Open(fileServer,fullentry.FullName(),EFileWrite) == KErrNone)
			{   
				TBuf8<256> savedTorrent;
				tempFile.Read(savedTorrent);
				TBuf16<256> tempBuf;
				tempBuf.Copy(savedTorrent);   
				LWRITELN(iLog, _L("NotifyOpen:"));
				
				TInt res;
				if ((tempBuf.Length() >= 9) && (tempBuf.Right(9) == _L(".storrent")))
					res = iTorrentMgr->OpenSavedTorrentL(tempBuf);
				else
					res = iTorrentMgr->OpenTorrentL(tempBuf);  
							
				if (res == KErrNone)
				{
					SetSelectedTorrentIndex(TORRENTMGR->TorrentCount() - 1);
					ActivateFilesViewL();
				}
							
				tempFile.Close();
			}      
			fileServer.Delete(fullentry.FullName());
		}
	}

	if (file_list!=NULL)
	delete file_list;

	fileServer.Close();
}

// Shows a message in a pop-up window
void CSymTorrentAppUi::ShowMessageL(TBuf<256> msg)
{
    CAknWarningNote* warningNote;
	warningNote = new (ELeave) CAknWarningNote;
    warningNote->ExecuteLD(msg);
}

#ifdef EKA2
void CSymTorrentAppUi::OpenFileL(const TDesC& aFileName)
{
	TDataType dataType; //= TDataType( KNullUid );
	TUid appUid;
	RFile sharableFile;

	if (iDocHandler == NULL)
	{
		iDocHandler = CDocumentHandler::NewL();
		iDocHandler->SetExitObserver( this );	
	}
		
	iDocHandler->OpenTempFileL( aFileName, sharableFile );
	CleanupClosePushL( sharableFile );
 
	RApaLsSession apaSession;
	CleanupClosePushL(apaSession);
	User::LeaveIfError(apaSession.Connect());

	User::LeaveIfError(apaSession.AppForDocument(sharableFile, appUid, dataType));
		    
	CAiwGenericParamList& paramList = iDocHandler->InParamListL();
	User::LeaveIfError(iDocHandler->OpenFileEmbeddedL( sharableFile, dataType, paramList ));				

	CleanupStack::PopAndDestroy(&apaSession);   // closes appArc
	CleanupStack::PopAndDestroy(&sharableFile); // closes sharableFile
}

void CSymTorrentAppUi::HandleServerAppExit( TInt aReason )
{
	delete iDocHandler;
	iDocHandler = NULL;

	// To handle 'Exit' correctly
	MAknServerAppExitObserver::HandleServerAppExit( aReason ); 
}


#else

void CSymTorrentAppUi::OpenFileL(const TDesC& aFileName)
{
	TDataType dataType; //= TDataType( KNullUid );
	TUid appUid;

	RApaLsSession apaLsSession;
	User::LeaveIfError(apaLsSession.Connect());

	// Identify file type. Sisx packages need to be installed by spawning a new thread. 
	RApaLsSession apaSession;
	CleanupClosePushL(apaSession);
	User::LeaveIfError(apaSession.Connect());
	
	User::LeaveIfError(apaSession.AppForDocument(aFileName, appUid, dataType));

	TThreadId threadId;	
	User::LeaveIfError(apaSession.StartDocument(aFileName, appUid, threadId));

	CleanupStack::PopAndDestroy(&apaSession);   // closes appArc
}

void CSymTorrentAppUi::NotifyExit(TExitMode /* aMode */)
{
}


#endif // EKA2

void CSymTorrentAppUi::OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection, TInt aConnectionIndex)
{		
#ifdef TRACKER_BUILD	
	if ((aConnectionIndex == 0) && (aResult))
	{
		if (iStartTrackerIssued)
		{
			iTrackerManager->StartTrackerL(&aConnection.BaseConnection());
			iTrackerManager->NetworkConnectionStartedL(aResult, aConnection.BaseConnection());			
			iStartTrackerIssued = EFalse;
		}
	}	
#endif
	
	if (!aResult && (aConnectionIndex == 0))
		(new (ELeave) CAknErrorNote)->ExecuteLD(_L("Failed to start network connection!"));
}

void CSymTorrentAppUi::NetworkManagerEventReceivedL(TKiNetworkManagerEvent /*aEvent*/)
{
}

void CSymTorrentAppUi::OnNetworkConnectionDownL(CNetworkConnection& /*aConnection*/, TInt /*aConnectionIndex*/)
{
	(new (ELeave) CAknErrorNote)->ExecuteLD(_L("Network disconnected!"));
}


#ifdef TRACKER_BUILD

void CSymTorrentAppUi::OnTrackerViewChangedL(TInt aViewId)
{
	SetCurrentViewId(aViewId);
}
		
void CSymTorrentAppUi::TrackerChangeNaviPaneTextL(const TDesC& aText)
{
	SetNaviPaneTextL(aText);
}
		
void CSymTorrentAppUi::TrackerRemoveNaviPaneTextL()
{
	RemoveNaviPaneText();
}
		
void CSymTorrentAppUi::TrackerActiveViewL(TSymTorrentTrackerViewIds aViewId)
{
	if (aViewId == ESymTorrentTrackerSettingsView)
		ActivateSettingsViewL();
	else if (aViewId == ESymTorrentTrackerMainView)
		ActivateMainViewL();
	else
		ActivateLocalViewL(TUid::Uid(aViewId));
}

// Initialize and start the tracker
void CSymTorrentAppUi::TrackerCmdStartTracker()
{
	if (iTorrentMgr->NetworkManager()->IsNetworkConnectionStarted(0))
	{
		iTrackerManager->StartTrackerL(&iTorrentMgr->NetworkManager()->Connection(0).BaseConnection());				
	}
	else
	{
		iStartTrackerIssued = ETrue;
		iTorrentMgr->NetworkManager()->StartNetworkConnectionL();
	}				
}


/**
* Activates the TorrentMakerFileListView
*/
void CSymTorrentAppUi::ActivateTorrentMakerFileListView()
{
	ActivateLocalViewL(TUid::Uid(ESymTorrentMakerFileListView));
}

void CSymTorrentAppUi::ActivateTrackerTorrentListViewL()
{
	DeactivateMainTabDecorator();
    ActivateLocalViewL(TUid::Uid(ESymTorrentTrackerTorrentListView));
    DisableNaviViewChange();
}

TBool CSymTorrentAppUi::InTorrentMaker() const
{
	return iTrackerManager->InTorrentMaker();
}

TBool CSymTorrentAppUi::InTracker() const
{
	return iTrackerManager->InTracker();
}

#endif


// End of File  
