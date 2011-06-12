/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrentEngine
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
 *  Name     : CSTTorrentManager from STTorrentManager.cpp
 *  Part of  : SymTorrent Engine
 *  Created  : 04.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STTorrentManager.h"
#include "STGlobal.h"
#include "STTorrent.h"
#include "STPreferences.h"
#include "STPeer.h"
#include "STPiece.h"
#include "STPeerConnection.h"
#include "STUtils.h"
#include "SymTorrentEngineLog.h"
#include <e32math.h>
#include <badesca.h>

#ifdef USE_DHT
#include "BitTorrentDHT.h"
#endif

#ifdef EKA2
	#ifdef __WINS__
 		_LIT(KSTLogFile, "c:\\SymTorrent.log");
 	#else
 		_LIT(KSTLogFile, "e:\\SymTorrentLOG.txt");
 	#endif
#else
 _LIT(KSTLogFile, "c:\\SymTorrent.log");
#endif


#ifdef __WINS__
const TInt KCloseTimeout = 30;
#else
const TInt KCloseTimeout = 10;
#endif

const TInt KSaveSettingsInterval = 40;

// ================= MEMBER FUNCTIONS =======================

EXPORT_C CSTTorrentManager* CSTTorrentManager::NewL()
{
	CSTTorrentManager* self = CSTTorrentManager::NewLC();
	CleanupStack::Pop();
	
	return self;	
}
	
	
EXPORT_C CSTTorrentManager* CSTTorrentManager::NewLC()
{
	CSTTorrentManager* self = new (ELeave) CSTTorrentManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	
	return self;	
}

void CSTTorrentManager::GeneratePeerIdL()
{
	iPeerId.Append(SYMTORRENT_PEER_ID_VERSION);
	
	for (TInt i=0; i<12; i++)	
		iPeerId.Append(TChar( TUint(Math::Rand(iSeed)) % 25 + 97 )); // random smaller case alphabatic characters ('a' - 'z')
}


CSTTorrentManager::CSTTorrentManager()
{
}
 
void CSTTorrentManager::ConstructL()
{
	#ifdef LOG_TO_FILE
	CKiLogManager::InitializeL();
	iLog = LOGMGR->CreateLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID), KSTLogFile);
	iLog->WriteLineL(_L("****************************************************"));
	iLog->WriteLineL(_L(" [SymTorrent] Engine starting"));
	iLog->WriteLineL(_L("****************************************************"));
	#endif
	
	SetLocalCooperationEnabled(EFalse);
	SetLocalConnectionMSType(EUndefined);

	// seed for random numbers	
	TTime now;
	now.HomeTime();
	iSeed = now.Int64();

	User::LeaveIfError(iFs.Connect());
	
	iDelayedTorrents = new (ELeave) CDesCArrayFlat(3);
	
	// Generatin random key
	iKey = TUint(Math::Rand(iSeed));

	GeneratePeerIdL();

	LWRITELN(iLog, _L("Loading preferences..."));
	iPreferences = new (ELeave) CSTPreferences;
	iPreferences->ConstructL();	
	iPreferences->AddPreferencesObserverL(this);

	iNetworkManager = NETWORKMGR;
	iNetworkManager->SetLog(iLog);
	/*iNetworkManager->Initialize(iPreferences->ProxyConnectionPort(), 
		iPreferences->ProxyServicePort(), 
		iPreferences->ProxyHostName(),
		iPreferences->IncomingPort(),
		iPreferences->AccessPointId());*/		
	iNetworkManager->AddObserverL(this);
	iNetworkManager->AddSocketAccepterL(this);
			
	// local connection, index = 1
	iNetworkManager->CreateNetworkConnectionL(CNetworkConnection::ERConnectionBased);
	
#ifdef USE_DHT
	// DHT LOG	
	#ifdef __WINS__
	iDHTLog = LOGMGR->CreateLoggerL(TUid::Uid(0xA00F20F5), _L("C:\\ST_DHT_LOG.txt"));
	#else
	iDHTLog = LOGMGR->CreateLoggerL(TUid::Uid(0xA00F20F5), _L("E:\\ST_DHT_LOG.txt"));
	#endif
	
	// DHT
	iDHT = new (ELeave) NKademlia::CBitTorrentDHT(iDHTLog, 777);
	iDHT->ConstructL("C:\\SymTorrentDHT.dat");
#endif
	
    iTimer = CPeriodic::NewL(CActive::EPriorityStandard + 1); // higher than standard priority
	// the timer ticks in every second
    iTimer->Start(1000000,1000000,
		TCallBack(StaticOnTimerL, this));

    LWRITE(iLog, _L("Peer ID: "));
    LWRITELN(iLog, iPeerId);	
}

EXPORT_C void CSTTorrentManager::InitializeGridTorrentEngineL()
{
}

EXPORT_C CSTTorrentManager::~CSTTorrentManager()
{
	iTorrentObservers.Reset();

#ifdef USE_DHT
	delete iDHT;
#endif
	
	delete iCloseTimer;
	iIncomingPeers.ResetAndDestroy();
	delete iTimer;
	iTorrents.ResetAndDestroy();
	delete iPreferences;
	iFs.Close();
	delete iDelayedTorrents;
	
	#ifdef LOG_TO_FILE
		CKiLogManager::Free();
	#endif
}

EXPORT_C TInt CSTTorrentManager::OpenSavedTorrentL(const TDesC& aFileName, CSTTorrent*& aTorrent)
{
	aTorrent = NULL;
	
	CSTTorrent* torrent = new (ELeave) CSTTorrent(this);
	CleanupStack::PushL(torrent);
	torrent->ConstructL();
	TInt loadResult = torrent->LoadSTorrentL(aFileName);
	
	if (loadResult == KErrNone)
	{
		// TBool startSeeding = torrent->IsComplete();
		loadResult = OpenTorrent2L(torrent, EFalse);
		if (loadResult != KErrNone)
			CleanupStack::PopAndDestroy(); // torrent
		else
		{
			aTorrent = torrent;
		//	NotifyTorrentObserverL(aTorrent, ESTEventTorrentOpened);
		}			
	}
	else
		CleanupStack::PopAndDestroy(); // torrent
		
	return loadResult;
}

EXPORT_C TInt CSTTorrentManager::OpenSavedTorrentL(const TDesC& aFileName)
{
	CSTTorrent* torrent = NULL;
	return OpenSavedTorrentL(aFileName, torrent);
}

EXPORT_C void CSTTorrentManager::LoadSavedTorrentsL()
{
	LWRITELN(iLog, _L("Loading saved torrents:"));
	const CDesCArrayFlat* savedTorrents = iPreferences->STorrents();
	
	for (TInt i=0; i<savedTorrents->Count(); i++)
	{
		LWRITELN(iLog, (*savedTorrents)[i]);
		
		if (OpenSavedTorrentL((*savedTorrents)[i]) != KErrNone)
		{
			iPreferences->RemoveSTorrentL((*savedTorrents)[i]);
			i--;
		}		
	}
}


EXPORT_C void CSTTorrentManager::OpenTorrentDelayedL(const TDesC& aFileName)
{
	iDelayedTorrents->AppendL(aFileName);
}


EXPORT_C TInt CSTTorrentManager::OpenTorrentL(const RFile& aFile)
{
	CSTTorrent* torrent = new (ELeave) CSTTorrent(this);
	CleanupStack::PushL(torrent);
	torrent->ConstructL();
	TInt loadResult = torrent->LoadL(aFile);

	loadResult = OpenTorrent2L(torrent);
	if (loadResult != KErrNone)
		CleanupStack::PopAndDestroy(); // torrent
	return loadResult;
}


EXPORT_C TInt CSTTorrentManager::OpenTorrentL(const TDesC& aFileName)
{
	CSTTorrent* torrent = NULL;
	return OpenTorrentL(aFileName, torrent);
}


EXPORT_C TInt CSTTorrentManager::OpenTorrentL(const TDesC& aFileName, CSTTorrent*& aTorrent)
{
	//TInt i = aFileName.Length()-1;
	//while ((i >= 0) && (aFileName[i] != '\\'))
	//	i--;
	//	
	//if ((aFileName.Length() - i - 1) <= 0)
	//	return KErrCopyFailed;
	//
	//TPtrC fileName = aFileName.Right(aFileName.Length() - i - 1);	
	//HBufC* fullFileName = HBufC::NewLC(fileName.Length()  + 
	//	Preferences()->TorrentPath().Length() + 10);
	//TPtr fullFileNamePtr(fullFileName->Des());
	//fullFileNamePtr.Copy(Preferences()->TorrentPath());
	//fullFileNamePtr.Append(fileName);

	//// appending a random tail to the filename
	//TBuf<20> randomTail;
	//randomTail.Num(TUint(Math::Rand(iSeed)) % 999999);
	//fullFileNamePtr.Append('-');
	//fullFileNamePtr.Append(randomTail);
	//
	//iFs.MkDirAll(*fullFileName);		
	//
	//TInt result;
	//{
	//	CFileMan* fileman = CFileMan::NewL(iFs);
	//	CleanupStack::PushL(fileman);
	//	result = fileman->Copy(aFileName, *fullFileName, 0);
	//	CleanupStack::PopAndDestroy(); // fileman		

	//	if (result != KErrNone)
	//	{		
	//		CleanupStack::PopAndDestroy(); // fullFileName;
	//		iLog->WriteLineL(_L("Copy failed!"));
	//		return KErrCopyFailed;
	//	}
	//}

	CSTTorrent* torrent = new (ELeave) CSTTorrent(this);
	CleanupStack::PushL(torrent);
	torrent->ConstructL();
	TInt loadResult = torrent->LoadL(aFileName);

	//	iLog->WriteLineL(*fullFileName);
	//	iFs.Delete(*fullFileName);
		//CFileMan* fileman = CFileMan::NewL(iFs);
		//CleanupStack::PushL(fileman);
		//result = fileman->Delete(*fullFileName, 0);
		//CleanupStack::PopAndDestroy(); // fileman
		
	//CleanupStack::PopAndDestroy(); // fullFileName;
	
	if (loadResult != KErrNone)
	{
		CleanupStack::PopAndDestroy(); // torrent
		return loadResult;		
	}

	loadResult = OpenTorrent2L(torrent, EFalse);
	if (loadResult != KErrNone)
		CleanupStack::PopAndDestroy(); // torrent
	else
		aTorrent = torrent;
		
	return loadResult;
}


EXPORT_C void CSTTorrentManager::RemoveTorrentL(CSTTorrent* aTorrent, TBool aDeleteIncompleteFiles)
{
	TInt index = iTorrents.Find(aTorrent);
	if (index >= 0)
		RemoveTorrentL(index, aDeleteIncompleteFiles);
}


EXPORT_C void CSTTorrentManager::RemoveTorrentL(TInt aIndex, TBool aDeleteIncompleteFiles)
{
	// deleting saved information
	if (iTorrents[aIndex]->SavedTorrent() != KNullDesC)
	{
		Fs().Delete(iTorrents[aIndex]->SavedTorrent());
		Preferences()->RemoveSTorrentL(iTorrents[aIndex]->SavedTorrent());
	}
	
	
	iTorrents[aIndex]->CloseL(aDeleteIncompleteFiles);
	/*
	if (iTorrentManagerObserver)
		iTorrentManagerObserver->RemoveTorrentL(aIndex);		
	
	//iTorrents[aIndex]->StopL();

	delete iTorrents[aIndex];
	iTorrents.Remove(aIndex);*/
}


EXPORT_C void CSTTorrentManager::CloseAllTorrentsL(MSTCloseTorrentObserver* aObserver)
{
	if ((iCloseTimer == 0) || (!iCloseTimer->IsActive()))
	{
		if ((iTorrents.Count()) == 0 && (aObserver))
			aObserver->TorrentsClosedL();
		else
		{		
			iCloseTorrentObserver = aObserver;
			
			delete iCloseTimer;
			iCloseTimer = 0;
		
			for (TInt i=0; i<iTorrents.Count(); i++)
				iTorrents[i]->CloseL();
			
			iCloseTimeoutCounter = KCloseTimeout;
			iCloseTimer = CPeriodic::NewL(0); // neutral priority
			// the timer ticks in every second
	    	iCloseTimer->Start(0,1000000,
				TCallBack(StaticOnCloseTimerL, this));
		}			
	}	
}


/*EXPORT_C void CSTTorrentManager::SetTorrentManagerObserverL(MSTTorrentManagerObserver* aObserver) 
{
	iTorrentManagerObserver = aObserver;

	if (iTorrentManagerObserver)
		for (TInt i=0; i<iTorrents.Count(); i++)
			NotifyObserverInsertL(iTorrents[i]);
}


EXPORT_C void CSTTorrentManager::NotifyObserverInsertL(CSTTorrent* aTorrent)
{
	if (!iTorrentManagerObserver)
		return;

	TInt index = iTorrents.Find(aTorrent);
	if (index >= 0)
	{
		iTorrentManagerObserver->InsertTorrentL(index, aTorrent->Name(), aTorrent->ConnectionCount(), 
			0, 0, aTorrent->DownloadPercent(), aTorrent->AvarageBytesPerSecond() / 1000, 
			aTorrent->AvarageUploadedBytesPerSecond() / 1000, aTorrent->LastStatus(), aTorrent->IsActive());
	}
}


EXPORT_C void CSTTorrentManager::NotifyObserverL(CSTTorrent* aTorrent, TSTTorrentStatus aEvent)
{
	if (iTorrentManagerObserver == 0)
		return;
	
//	iLog->WriteLineL(_L("TorrentManagerObserver is set"));
//	iLog->WriteLineL(TInt(aEvent));

	TInt index = iTorrents.Find(aTorrent);
	if (index >= 0)
	{
	//	iLog->WriteLineL(_L("Found torrent"));
		aTorrent->SetLastStatus(aEvent);
		iTorrentManagerObserver->ModifyTorrentL(index, aTorrent->Name(), aTorrent->ConnectionCount(), 
			0, 0, aTorrent->DownloadPercent(), aTorrent->AvarageBytesPerSecond() / 1000, 
			aTorrent->AvarageUploadedBytesPerSecond() / 1000, aEvent, aTorrent->IsActive());
	}
}*/


EXPORT_C void CSTTorrentManager::SaveTorrentsStateL()
{
	for (TInt i=0; i<iTorrents.Count(); i++)
		iTorrents[i]->SaveStateL();
}


EXPORT_C TInt CSTTorrentManager::AttachPeerToTorrentL(const TDesC8& aInfoHash, const TDesC8& aPeerId, CSTPeerConnection* aPeerConnection)
{
	for (TInt i=0; i<iTorrents.Count(); i++)
	{
		CSTTorrent* torrent = iTorrents[i];
		if (torrent->InfoHash() == aInfoHash)
		{
			// check whether the torrent is being shared (is active)
			if (!torrent->IsActive())
				return KErrGeneral;
			
			if (torrent->ConnectionCount() >= (KMaxPeerConnectionCount + KIncomingConnectionSlots))
				return KErrGeneral;

			CSTPeer* peer = torrent->GetPeer(aPeerId);
			if (peer && (peer->IsLocal() == aPeerConnection->Peer()->IsLocal()))
			{				
				if ((peer->State() == EPeerNotConnected) || (peer->State() == EPeerClosing))				
				{	
					// deletes the dummy incoming peer object
					CSTPeer* dummyPeer = aPeerConnection->Peer();
					dummyPeer->DetachConnection();
					iIncomingPeers.Remove(iIncomingPeers.Find(dummyPeer));
					delete dummyPeer;
					
					peer->AttachConnectionL(aPeerConnection);
					aPeerConnection->SetTorrent(torrent);
					
					
					//torrent->DeletePeer(aPeerConnection->RemoteAddress());
					
					//aPeerConnection->Peer()->SetBitFieldLengthL(torrent->PieceCount());

					
					//if (torrent->AddPeerL(aPeerConnection->Peer()) != KErrNone)
					//	return KErrGeneral;
					

					
					return KErrNone;
				}
				else
					return KErrGeneral; // peer is already connected
			}
			else
			{
				// delete primary peer
				if (peer && peer->Connection())
					peer->Connection()->CloseL(CSTPeerConnection::EDeletePeer);
				
				aPeerConnection->SetTorrent(torrent);
				aPeerConnection->Peer()->SetBitFieldLengthL(torrent->PieceCount());
				
				if (torrent->AddPeerL(aPeerConnection->Peer()) != KErrNone)
					return KErrGeneral;
				
				iIncomingPeers.Remove(iIncomingPeers.Find(aPeerConnection->Peer()));
					
				return KErrNone;
			}
		}
	}
	
	return KErrGeneral;
}


EXPORT_C void CSTTorrentManager::RegisterIncomingConnectionL(RSocket* aSocket, TBool aIsLocal)
{
	LWRITELN(iLog, _L("[TorrentMgr] RegisterIncomingConnectionL"));
	CSTPeer* peer = CSTPeer::NewLC(aSocket, this, aIsLocal);
	User::LeaveIfError(iIncomingPeers.Append(peer));
	CleanupStack::Pop(); // peer
}


EXPORT_C void CSTTorrentManager::SetEngineStateObserverL(MSTEngineStateObserver* aObserver)
{	
	if (iEngineStateObserver == NULL)
	{
		iEngineStateObserver = aObserver;
		NotifyEngineStateObserverL();
	}			
}

EXPORT_C void CSTTorrentManager::NotifyEngineStateObserverL()
{
	if (iEngineStateObserver)
	{
		TInetAddr addr;
		iNetworkManager->GetLocalAddress(0, addr);
		
		iEngineStateObserver->EngineStateChangedL(addr, 
			iIncomingConnectionCount, iBytesDownloaded, iBytesUploaded);		
	}
}

EXPORT_C void CSTTorrentManager::SetAllTorrentsFailedL()
{
	for (TInt i=0; i<iTorrents.Count(); i++)
	{
		iTorrents[i]->SetFailedL();
	}
}

TInt CSTTorrentManager::StaticOnTimerL(TAny* aObject)
{
	// cast, and call non-static function
	((CSTTorrentManager*)aObject)->OnTimerL();
	return 1;
}

TInt CSTTorrentManager::OpenTorrent2L(CSTTorrent* aTorrent, TBool aStart)
{
	TInt loadResult = KErrNone;

	// checks if the torrent is already opened
	for (TInt i=0; i<iTorrents.Count(); i++)
	{			
		if (iTorrents[i]->InfoHash() == aTorrent->InfoHash())				
		{
			loadResult = KErrAlreadyOpened;
			break;
		}
	}

	if (loadResult == KErrNone)
	{
		User::LeaveIfError(iTorrents.Append(aTorrent));
		CleanupStack::Pop(); // torrent

	//	NotifyObserverInsertL(aTorrent);
		
		NotifyTorrentObserverL(aTorrent, ESTEventTorrentOpened);

		if (aStart)
			aTorrent->StartL();
		
		Preferences()->SaveSettingsL(); // New torrent added -> save to the settings file
	}

	return loadResult;
}

void CSTTorrentManager::OnTimerL()
{
	// incresing counter
	iEllapsedTime++;
	
	if (iEllapsedTime == 1)
	{
		//LoadSavedTorrentsL();
		
		//iNetworkManager->StartNetworkConnectionL(); //////////////////////////// HACKKKKKKKK
		
		if (iDelayedTorrents->Count() > 0)
		{
			for (TInt i=0; i<iDelayedTorrents->Count(); i++)
				OpenTorrentL((*iDelayedTorrents)[i]);	
			iDelayedTorrents->Reset();
		}
	}
	else
		if ((iEllapsedTime % KSaveSettingsInterval) == 0)
		{
			if (!iCloseTimer) // only save settings if the application is not exiting
			{
				SaveTorrentsStateL();
				Preferences()->SaveSettingsL();
			}			
		}
		
	
/*	if ((!iNetworkManager->IsNetworkConnectionStarted()) && (iPreferences->AccessPointId() != 0))
	{
  		iNetworkManager->StartNetworkConnectionL(iPreferences->AccessPointId());
	}*/
	TInt activeTorrentCount = 0;
	
	TInt i;
	for (i=0; i<iTorrents.Count(); i++)
	{
		CSTTorrent* torrent = iTorrents[i];
		torrent->OnTimerL();
				
		if (torrent->IsClosed()) // deleting closed torrent
		{
			//if (iTorrentManagerObserver)
			//	iTorrentManagerObserver->RemoveTorrentL(i);
			
			NotifyTorrentObserverL(torrent, ESTEventTorrentClosed);
			
			delete iTorrents[i];
			iTorrents.Remove(i);
			i--;
			
			Preferences()->SaveSettingsL(); // Torrent removed, save status to settings file
		}
		else
			if (torrent->IsActive())
				activeTorrentCount++;
	}
	
	for (i=0; i<iIncomingPeers.Count(); i++)
		iIncomingPeers[i]->OnTimerL(NULL, iEllapsedTime);
		
	for (i=0; i<iIncomingPeers.Count(); i++)
	{	
		CSTPeer* peer = iIncomingPeers[i];	
		if ((peer->State() == EPeerClosing) || (peer->State() == EPeerNotConnected))
		{
			delete peer;
			iIncomingPeers.Remove(i);
			i--;
		}
	}
	
	if ((activeTorrentCount == 0) && (iPreferences->CloseConnectionAfterDownload()))
	{
		if (iNetworkManager->IsListening(0))
		{
			iNetworkManager->StopAllListening(0);
			iIncomingPeers.ResetAndDestroy();			
		}
		
		if (iNetworkManager->IsNetworkConnectionStarted(0))
		{
			//iLog->WriteLineL(_L("[TorrentMgr] Stopping listening"));
			iNetworkManager->CloseNetworkConnection();
		}		
	}
		
}

TInt CSTTorrentManager::StaticOnCloseTimerL(TAny* aObject)
{
	// cast, and call non-static function
	((CSTTorrentManager*)aObject)->OnCloseTimerL();
	return 1;
}
	
void CSTTorrentManager::OnCloseTimerL()
{
	if (iCloseTimeoutCounter > 0)
	{
		TBool allTorrentsClosed = ETrue;
		for (TInt i=0; i<iTorrents.Count(); i++)
		{
			if (!iTorrents[i]->IsClosed())
			{
				allTorrentsClosed = EFalse;
				break;
			}
		}
		
		if (allTorrentsClosed)
		{
			LWRITELN(iLog, _L("[TorrentMgr] All torrents"));
			iCloseTimer->Cancel();
			
			if (iCloseTorrentObserver)
				iCloseTorrentObserver->TorrentsClosedL();
		}
		else
			iCloseTimeoutCounter--;
	}
	else
	{
		LWRITELN(iLog, _L("[TorrentMgr] Close timeout, forced exit"));
		iCloseTimer->Cancel();
		
		if (iCloseTorrentObserver)
			iCloseTorrentObserver->TorrentsClosedL();
	}
}

EXPORT_C void CSTTorrentManager::AddTorrentObserverL(	MSTTorrentObserver* aObserver,
														CSTTorrent* aTorrent,
														TUint32 iEventFlags)
{
	for (TInt i=0; i<iTorrentObservers.Count(); i++)
		if (iTorrentObservers[i].iObserver == aObserver) return;
		
	TSTTorrentObserver observer(aObserver, iEventFlags, aTorrent);
	User::LeaveIfError(iTorrentObservers.Append(observer));
}

EXPORT_C void CSTTorrentManager::RemoveTorrentObserver(MSTTorrentObserver* aObserver)
{
	for (TInt i=0; i<iTorrentObservers.Count(); i++)
		if (iTorrentObservers[i].iObserver == aObserver)
		{
			iTorrentObservers.Remove(i);
			break;
		}
}


void CSTTorrentManager::NotifyTorrentObserverL(CSTTorrent* aTorrent, TUint32 aEventFlags)
{
	TInt index = iTorrents.Find(aTorrent);
	
	for (TInt i=0; i<iTorrentObservers.Count(); i++)
	{
		if ((iTorrentObservers[i].iTorrent == 0) || (iTorrentObservers[i].iTorrent == aTorrent))
			if ((iTorrentObservers[i].iEventFlags & aEventFlags) > 0)
				iTorrentObservers[i].iObserver->TorrentChangedL(aTorrent, index, aEventFlags);
	}
}

void CSTTorrentManager::SettingChangedL(TSymTorrentSetting aSetting)
{
	CSTPreferences* prefs = iPreferences;

	/*if (aSetting == ESettingProxyConnectionPort)
	{
		if (iNetworkManager->ProxyParameters()->GetConnectionPort() != prefs->ProxyConnectionPort())
		{
			iNetworkManager->ProxyParameters()->SetConnectionPort(prefs->ProxyConnectionPort());
			
			if ((prefs->IncomingConnectionsMode() == EEnabledWithProxy) &&
				(iNetworkManager->ProxyConnectorState() != EConOffline))
			{
				iNetworkManager->DisconnectProxy();
				iNetworkManager->ConnectProxyL();
			}			
		}
	}
	else
	
	if (aSetting == ESettingProxyServicePort)
	{
		if (iNetworkManager->ProxyParameters()->GetServicePort() != prefs->ProxyServicePort())
		{
			iNetworkManager->ProxyParameters()->SetServicePort(prefs->ProxyServicePort());
			
			if ((prefs->IncomingConnectionsMode() == EEnabledWithProxy) &&
				(iNetworkManager->ProxyConnectorState() != EConOffline))
			{
				iNetworkManager->DisconnectProxy();
				iNetworkManager->ConnectProxyL();
			}			
		}
	}
	else
	
	if (aSetting == ESettingProxyHostName)
	{
		if (prefs->ProxyHostName() != iNetworkManager->ProxyParameters()->GetProxyAddress())
		{
			iNetworkManager->ProxyParameters()->SetProxyAddress(prefs->ProxyHostName());
			
			if ((prefs->IncomingConnectionsMode() == EEnabledWithProxy) &&
				(iNetworkManager->ProxyConnectorState() != EConOffline))
			{
				iNetworkManager->DisconnectProxy();
				iNetworkManager->ConnectProxyL();
			}
			
		}
		iNetworkManager->ProxyParameters()->SetProxyAddress(prefs->ProxyHostName());	
	}
	else*/
	
	if (aSetting == ESettingIncomingPort)
	{				
		if ((iNetworkManager->IsListening(0)) && (!iNetworkManager->IsListening(0, prefs->IncomingPort())))
		{
			iNetworkManager->StopAllListening(0);
			iNetworkManager->StartListeningL(0, prefs->IncomingPort());
		}				
	}
	else
	
	if (aSetting == ESettingAccesPointId)
	{
		CNetworkConnection& conn = iNetworkManager->Connection(0);
		
		if ((prefs->AccessPointId() == -1) && (conn.IsInitialized()))
		{
			conn.SetUninitialized();
		}
		else
			if ((prefs->AccessPointId() == KBluetoothIapId) && (conn.Type() != CNetworkConnection::EBluetooth))
			{
				conn.SetUninitialized();
				conn.InitializeL(CNetworkConnection::EBluetooth);
			}
			else
				if ((conn.Type() != CNetworkConnection::ERConnectionBased) ||
					(conn.IapId() != prefs->AccessPointId()))
				{
					conn.SetUninitialized();
					conn.InitializeL(CNetworkConnection::ERConnectionBased, prefs->AccessPointId());
				}							
	}
	else
		
	if (aSetting == ESettingDHTEnabled)
	{
	#ifdef USE_DH
		if (prefs->IsDHTEnabled() && iNetworkManager->IsNetworkConnectionStarted())
		{
			iDHT->StartL();		
		}
		else
			iDHT->StopL();
	#endif
	}
	else
	
	if (aSetting == ESettingIncomingConnectionsMode)
	{
		iNetworkManager->SetIncomingConnectionModeL(prefs->IncomingConnectionsMode());				
	}		
}

TBool CSTTorrentManager::AcceptSocketL(RSocket* aSocket, TUint /*aPort*/, CNetworkConnection& aConnection, TInt aConnIndex)
{
	TBool isLocal = (aConnIndex == 1);
	
	if (isLocal && (aConnection.Type() == CNetworkConnection::ERConnectionBased))
	{
		TInetAddr localAddress;
		iNetworkManager->GetLocalAddress(aConnIndex, localAddress);
		
		TInetAddr remoteAddress;
		aSocket->RemoteName(remoteAddress);
		
		if (localAddress.Address() == remoteAddress.Address())
		{
			LWRITELN(iLog, _L("[TorrentMgr] Connected to ourselves via local connection, closing"));
			return EFalse;
		}
	}
	
	RegisterIncomingConnectionL(aSocket, isLocal);	
	
	return ETrue;
}

void CSTTorrentManager::NetworkManagerEventReceivedL(TKiNetworkManagerEvent /*aEvent*/)
{
	/*switch (aEvent)
	{
		case EKiEventProxyConnected:
		case EKiEventProxyDisconnected:
		case EKiEventListeningStarted:
		case EKiEventListeningStopped:
	}*/
	
	NotifyEngineStateObserverL();
}

void CSTTorrentManager::CloseNetworkIfNoTorrentsActiveL()
{
	TBool allClosed = ETrue;
	
	for (TInt i=0; i<iTorrents.Count(); i++)
	{
		if (iTorrents[i]->IsActive())
		{
			allClosed = EFalse;
			break;
		}
	}
	
	if (allClosed)
		iNetworkManager->CloseNetworkConnection();
}

void CSTTorrentManager::OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& /*aConnection*/, TInt aConnectionIndex)
{
	if (aConnectionIndex == 0)
	{	
		if (aResult)
		{
			#ifdef USE_DHT
				iDHT->StartL();
			#endif
		}
		else
		{
			for (TInt i=0; i<iTorrents.Count(); i++)
				iTorrents[i]->StopL();
		}	
	}
	
	if ((aConnectionIndex == 1) && aResult)
	{
		if (iNetworkManager->Connection(1).Type() == CNetworkConnection::EBluetooth)
			iLocalConnMasterSlaveMode = ETrue;
	}
}	
	
void CSTTorrentManager::OnNetworkConnectionDownL(CNetworkConnection& /*aConnection*/, TInt /*aConnectionIndex*/)
{
	//TODO display error on UI, stop connection, etc...

	for (TInt i=0; i<iTorrents.Count(); i++)
		iTorrents[i]->StopL();
}

void CSTTorrentManager::OnLocalUDPReceiveL(TInetAddr aSender, const TDesC8& aData)
{
	HLWRITELN(iLog, _L("[TorrentManager] OnLocalUDPReceiveL begin"));
	
	TInetAddr localAddress;
	iNetworkManager->GetLocalAddress(1, localAddress);
	
	if (localAddress.Address() == aSender.Address())
	{
		HLWRITELN(iLog, _L("[TorrentManager] Throwing away own message"));
		return;
	}
	
#ifdef LOG_TO_FILE
	LWRITE(iLog, _L("[TorrentManager] UDP sender: "));
	TBuf<128> addressBuf;
	if (aSender.Address() == 0)
	{
		addressBuf = _L("? (could not get local address)");
	}
	else
	{
		aSender.Output(addressBuf);
		addressBuf.Append(_L(":"));
		TBuf<16> portBuf;
		portBuf.Num(localAddress.Port());
		addressBuf.Append(portBuf);
	}
	HLWRITELN(iLog, addressBuf);
#endif
	
	//HLWRITEL(iLog, _L("[TorrentManager] Data received: "));
	//HLWRITELN(iLog, aData);
	
	// TODO handle multiple torrents
	if (iTorrents.Count() == 0)
		return;
	
	if (aData.Size() >= 4)
	{
		TUint messageLength = NSTUtils::ReadInt32(aData);
		
		LWRITE(iLog, _L("[TorrentManager] Datagram length: "));
		LWRITELN(iLog, aData.Size());
				
		LWRITE(iLog, _L("[TorrentManager] Message length: "));
		LWRITELN(iLog, messageLength);
		
		if ((TUint(aData.Size()) >= (4 + messageLength)) && 
			(aData[4] == KMessageIdPiece))
		{
			TInt index = NSTUtils::ReadInt32(aData.Mid(5));
			TInt begin = NSTUtils::ReadInt32(aData.Mid(9));
			
			TBool pendingRequestFound = EFalse;
			
			// check if the incoming local piece is requested by this peer
			for (TInt i=0; i<iTorrents[0]->PeerCount(); i++)
			{
				if ((iTorrents[0]->Peer(i)->IsLocal()) && (iTorrents[0]->Peer(i)->Connection()) && (iTorrents[0]->Peer(i)->Connection()->State() == EPeerPwConnected))
				{
					pendingRequestFound = iTorrents[0]->Peer(i)->Connection()->HandleIncomingLocalPieceL(index, begin, aData.Mid(13, messageLength - 9));
					if (pendingRequestFound)
						break;
				}
			}
			
			if (!pendingRequestFound)
			{
				if ((iTorrents[0]->PieceCount() > index) && (!iTorrents[0]->Piece(index)->IsDownloaded()))
				{
					HLWRITELN(iLog, _L("[TorrentManager] Received unrequested piece"));
					
					CSTPiece* piece = iTorrents[0]->Piece(index);
					CSTPeer* peer = iTorrents[0]->GetPeer(aSender);
					
					if (piece->InsertBlockL(begin, aData.Mid(13, messageLength - 9), peer) != KErrNone)
					{
						LWRITELN(iLog, _L8("CRITICAL FAULT, Writing to piece failed")); // CRITICAL FAULT								
					}
					else
					{
						HLWRITELN(iLog, _L("[TorrentManager] Writing piece complete"));
						
						if (iTorrents[0]->EndGame())									
							iTorrents[0]->EndGamePieceReceivedL(piece, peer);
					}
					
					iTorrents[0]->iLocalSubPiecesNotRequested++;
					iTorrents[0]->iLocalSubPiecesNotRequestedSize += aData.Size();
					
					
					// TODO remove commented part if the code above is working
					/*if (piece->DownloadedSize() == begin)
					{
						CSTPeer* peer = iTorrents[0]->GetPeer(aSender);
						
						if (piece->AppendBlockL(aData, peer) != KErrNone)
						{
							LWRITELN(iLog, _L8("CRITICAL FAULT, Writing to piece failed")); // CRITICAL FAULT								
						}
						else
						{
							HLWRITELN(iLog, _L("[TorrentManager] Writing piece complete"));
							
							if (iTorrents[0]->EndGame())									
								iTorrents[0]->EndGamePieceReceivedL(piece, peer);
						}
						
						iTorrents[0]->iLocalSubPiecesNotRequested++;
					}
					else
						iTorrents[0]->iLocalSubPiecesReceivedNotMatchPieceBeginning++;*/
				}
				else
				{
					iTorrents[0]->iLocalSubPiecesReceivedAlreadyDownloaded++;
					iTorrents[0]->iLocalSubPiecesReceivedAlreadyDownloadedSize += aData.Size();
				}
			}
		}
	}
	
	HLWRITELN(iLog, _L("[TorrentManager] OnLocalUDPReceiveL end"));
}

#ifndef EKA2
GLDEF_C TInt E32Dll( TDllReason )
{
    return KErrNone;
}
#endif
