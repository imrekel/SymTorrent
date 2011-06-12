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
 * along with SymTorrent; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/*
* ============================================================================
*  Name     : CSymTorrentAppUi from SymTorrentAppui.h
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
* ============================================================================
*/

#ifndef SYMTORRENTAPPUI_H
#define SYMTORRENTAPPUI_H

// INCLUDES
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <akntabobserver.h>
#include <aknprogressdialog.h> 
#include "SymTorrent.hrh"
#include "NetworkManager.h"
#include "STTorrentManager.h"
#include "STDefs.h"
#include "NotifyFileChange.h"

#include <apparc.h>
#include <documenthandler.h>

#ifdef EKA2
	#include <AknServerApp.h>

	class CSchemeHandler;
	class CDocumentHandler;
#endif
	
#ifdef TRACKER_BUILD
	#include "SymTorrentTrackerManager.h"	
#endif

// FORWARD DECLARATIONS
class CSymTorrentContainer;
class CSTTorrentManager;
class CAknTitlePane;
class CAknWaitDialog;
class CPeriodic;
class CAknNavigationControlContainer;
class CSymTorrentTrackerManager;
class CKiLogger;

// CONSTS
_LIT(KNotifyFile, "C:\\Data\\BitTorrent\\Notify\\STNotify.nt");
_LIT(KNotifyFileMod, "C:\\Data\\BitTorrent\\Notify\\STNotifyMod.nt");
_LIT(KNotifyDir, "C:\\Data\\BitTorrent\\Notify\\");

// CLASS DECLARATION

class MSymTorrentViewChanger // needed by tracker
{
public:
	virtual void ActivateViewL(TSymTorrentViewId aViewId) = 0;
	
	virtual void SetNaviPaneTextL(const TDesC& aText) = 0;
 	
 	virtual void RemoveNaviPaneText() = 0;
};

/**
 * Main application UI class
 */
class CSymTorrentAppUi : 	public CAknViewAppUi, 
							public MAknTabObserver, 
							public MKiAccessPointSupplier, 
							public MSTCloseTorrentObserver,
							public MProgressDialogCallback,
							public MKiNetworkManagerObserver,
							public MSTEngineEventObserver,
							#ifdef TRACKER_BUILD	
								public MTrackerUIConnection,
							#endif
							
							#ifdef EKA2
								public MAknServerAppExitObserver
							#else
								public MApaEmbeddedDocObserver
							#endif // EKA2
{
public: // Constructors and destructor

    void ConstructL();

    ~CSymTorrentAppUi();
    
    void ExitSymTorrentL();
    
public: // New functions
	
	/**
    * Shows and enables the naviPane
    */
	void EnableNaviViewChange();
	/**
    * Hides and dissables the naviPane
    */
	void DisableNaviViewChange();
	/**
    * Activates the DownloadState View
    */
	void ActivateDownloadStateViewL();
	/**
    * Activates the DetailView
    */
	void ActivateTorrentDetailViewL();
	/**
    * Activates the FilesView
    */
	void ActivateFilesViewL();
	/**
    * Activates the MainView
    */
	void ActivateMainViewL();

	/**
    * Activates the SettingsView
    */
	void ActivateSettingsViewL();
	
	/**
    * Sets the selected torrent index
    */
	void SetSelectedTorrentIndex(TInt aIndex);

	/**
    * Returns the selected torrent index
    */
	TInt SelectedTorrentIndex();
	
	void ActivateStatusViewL();
	
	inline void SetCurrentViewId(TInt aViewId);
	
	inline TInt CurrentViewId() const;
	
	inline CAknTitlePane* TitlePane() const;
	
	/**
     * Handles when a notify message arrives through a file
     */ 
 	void HandleNotifyMessageL();
 	
 	void SetNaviPaneTextL(const TDesC& aText);
 	
 	void RemoveNaviPaneText();
 	
 	CAknNavigationControlContainer* NaviPane();
 	
 	void OpenFileL(const TDesC& aFileName);
 	
 	/**
 	 * Text color from the UI theme
 	 */
 	inline TRgb TextColor() const;
 	
    /**
     * From CEikAppUi, takes care of command handling.
     * @param aCommand command to be handled
     */
    void HandleCommandL(TInt aCommand);
 	
private:

#ifdef EKA2
	// From MAknServerAppExitObserver
	void HandleServerAppExit(TInt aReason);
#else
	// From MApaEmbeddedDocObserver
	void NotifyExit(TExitMode aMode);
#endif // EKA2
 
#ifdef TRACKER_BUILD	 // Tracker
	public:

		void ActivateTrackerTorrentListViewL();

		/**
	    * Activate the TorrentMakerFileListView
	    */
		void ActivateTorrentMakerFileListView();
		
		void TrackerCmdStartTracker();
		
		TBool InTorrentMaker() const;
		
		TBool InTracker() const;
		
		inline CSymTorrentTrackerManager* TrackerManager();
		
	private:
			
		void OnTrackerViewChangedL(TInt aViewId);
		
		void TrackerChangeNaviPaneTextL(const TDesC& aText);
		
		void TrackerRemoveNaviPaneTextL();
		
		void TrackerActiveViewL(TSymTorrentTrackerViewIds aViewId);

	private:
	
		CSymTorrentTrackerManager*		iTrackerManager;
#endif

public: // from MSymTorrentViewChanger

	void ActivateViewL(TSymTorrentViewId aViewId);
 	
private: // from MKiNetworkManagerObserver

	// TODO these should be moved to the engine when the tracker has been moved to the engine
	void NetworkManagerEventReceivedL(TKiNetworkManagerEvent aEvent);
		
	void OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection, TInt aConnectionIndex);			
	
	void OnNetworkConnectionDownL(CNetworkConnection& aConnection, TInt aConnectionIndex);
	
private: // from MSTEngineEventObserver
	
	void HandleTorrentCriticalFaultL(CSTTorrent* aTorrent, TSTCriticalFaultType aFault);

public: // Functions from base classes

	/**
     * From MAknTabObserver.
     * @param aIndex tab index
     */
	void TabChangedL(TInt aIndex); 
		
private:

    // From MEikMenuObserver
    void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

	TBool ProcessCommandParametersL(TApaCommand aCommand,TFileName& aDocumentName,const TDesC8& aTail);
	
	void ProcessMessageL(TUid aUid,const TDesC8& aParams);



    /**
     * From CEikAppUi, handles key events.
     * @param aKeyEvent Event to handled.
     * @param aType Type of the key event. 
     * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
     */
    TKeyResponse HandleKeyEventL(
        const TKeyEvent& aKeyEvent,TEventCode aType);
        
    void InsertFileL(const TDesC& aData);
    
	TBool FileSelectQueryL(TFileName &aFileName);
	
	void DeactivateMainTabDecorator();
	
	void ActivateMainTabDecoratorL();
	
//	void ViewActivatedL(CAknView *aView, const TVwsViewId &aPrevViewId, TUid aCustomMessageId, const TDesC8 &aCustomMessage);
	
	void HandleResourceChangeL(TInt aType);

	void ShowMessageL(TBuf<256> msg);
	
	/**
	 * Opens a progress dialog and checks the given torrent's hash
	 */
	void CheckHashL(CSTTorrent* aTorrent);
	
private: // from MSTCloseTorrentObserver

	void TorrentsClosedL();
	
private: // from MProgressDialogCallBack

	void DialogDismissedL (TInt aButtonId); 
	
private: // from MAccesPointSupplier

    TBool GetIapIdL(TInt32& aAccesPointId, TDes& aAccesPointName, TInt aNetConnIndex);
	
private:	

	static TInt StaticOnCloseTimerL(TAny* aObject);
	
	void OnCloseTimerL();
	
	static TInt StaticOnStartupTimerL(TAny* aObject);
	
	void OnStartupTimerL();

private: //Data

	TInt							iCloseTimeoutCounter;

    CAknNavigationControlContainer* iNaviPane;
    
    CAknTabGroup*                   iTabGroup;
    
    CSTTorrentManager*				iTorrentMgr;
    
	CAknNavigationDecorator*	    iDecorator;
	
	TBool							isEnabledNaviViewChange;
	
	TFileSelectionType				iFileSelectionType;
	
	TInt							iSelectedTorrentIndex;
	
	CAknTabGroup*                   iMainTabGroup;
	
	CAknNavigationDecorator*	    iMainNaviDecorator;
	
	TBool							iMainNaviDecoratorActive;
	
	TInt							iCurrentViewId;
	
	CAknTitlePane* 					iTitlePane;
	
	CAknWaitDialog*					iWaitDialog;
	
	CAknProgressDialog*				iProgressDialog;
	
	CPeriodic*						iCloseTimer;
	
	CPeriodic*						iStartupTimer;
	
	TBool							iSavedBeforeExit;
	
	CNotifyFileChange*    			iNotifyFileChange;
	
	CAknNavigationDecorator* 		iLabelNaviDecorator;
	
	TBool                           iStartTrackerIssued;
	
	CKiLogger*						iLog;
	
	TInt							iSymTrackerResourceOffset;
	
	/**
	 * Default text color of the UI theme
	 */
	TRgb							iTextColor;
	
#ifdef EKA2
	CDocumentHandler* 				iDocHandler;
#else
//	CEikDocument*                   iDocument;
//	CEikProcess*                    iProcess;
#endif // EKA2
};
    
inline void CSymTorrentAppUi::SetCurrentViewId(TInt aViewId) {
	iCurrentViewId = aViewId;
}

inline CAknTitlePane* CSymTorrentAppUi::TitlePane() const {
	return iTitlePane;
}

inline TInt CSymTorrentAppUi::CurrentViewId() const {
	return iCurrentViewId;
}

inline TRgb CSymTorrentAppUi::TextColor() const {
	return iTextColor;
}

#ifdef TRACKER_BUILD
inline CSymTorrentTrackerManager* CSymTorrentAppUi::TrackerManager() {
	return iTrackerManager;
}
#endif

#endif

// End of File

