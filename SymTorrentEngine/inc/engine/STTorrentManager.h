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

#ifndef SYMTORRENT_STTORRENTMANAGER_H
#define SYMTORRENT_STTORRENTMANAGER_H

// INCLUDES
#include <e32base.h>
#include "NetworkManager.h"
#include "STPreferences.h"
#include <badesca.h>
#include "STDefs.h"
#include "STEnums.h"

// FORWARD DECLARATIONS
class CPeriodic;
class CSTTorrent;
class CSTPreferences;
class CKiLogger;
class CNetworkManager;
class CSTPeerConnection;
class CSTPeer;
class CSTPreferences;

#ifdef USE_DHT
namespace NKademlia { class CBitTorrentDHT; }
#endif

/**
 * 
 */
enum TSTTorrentStatusInfo
{
	ENotSpecified = 0,
	ETrackerConnecting,
	ETrackerSuccessful,
	ETrackerFailed,
	ETorrentFailed,
	ETorrentRemoving // closing
};

/**
 * The following flags may be combined with a bitwise OR
 */
enum TSTTorrentObserverEventFlags
{
	ESTEventTorrentOpened 			= 1,
	ESTEventTorrentClosed 			= 2,
	ESTEventConnectionsChanged 		= 4,
	ESTEventDownloadedBytesChanged 	= 8,
	ESTEventUploadedBytesChanged 	= 16,
	ESTEventDownloadSpeedChanged   	= 32,
	ESTEventUploadSpeedChanged		= 64,
	ESTEventStatusInfoChanged		= 128,
	ESTEventTorrentStopped			= 256,	// pause
	ESTEventTorrentStarted			= 1024,	// resume
	ESTEventPieceDownloaded			= 2048
};

const TUint32 KSTAllTorrentObserverEvents = 0xFFFFFFFF; // Masks all possible events


// CLASS DECLARATIONS


/**
 * MSTCloseTorrentObserver
 *
 * Called when the torrents have been closed
 */
class MSTCloseTorrentObserver
{
public:
	virtual void TorrentsClosedL() = 0;
};

/**
 * MSTTorrentObserver
 */
class MSTTorrentObserver
{
public:

	virtual void TorrentChangedL(CSTTorrent* aTorrent, TInt aIndex, TUint32 aEventFlags) = 0;
};

/**
 * TSTTorrentObserver
 *
 * Utility class for storing a pointer to an observer, the observed torrent and the 
 * list of events in which it is interested.
 */
class TSTTorrentObserver
{
public:

	TSTTorrentObserver(MSTTorrentObserver* aObserver, TUint32 aEventFlags, CSTTorrent* aTorrent) 
	 : iObserver(aObserver), iEventFlags(aEventFlags), iTorrent(aTorrent)
	 {	 	
	 }
	 
public:	 

	MSTTorrentObserver* iObserver;
	
	TUint32 iEventFlags;
	
	CSTTorrent* iTorrent;
};

/**
 * MSTEngineStateObserver
 *
 * Class for observing changes in the engine status
 */
class MSTEngineStateObserver
{
public:
	
	virtual void EngineStateChangedL(	TInetAddr aLocalAddress, 
										TInt aIncomingConnectionCount, 
										TInt aBytesDownloaded, 
										TInt aBytesUploaded) = 0;
};

/**
 * Observs engine events
 */
class MSTEngineEventObserver
{
public:
	
	virtual void HandleTorrentCriticalFaultL(CSTTorrent* aTorrent, TSTCriticalFaultType aFault) = 0;
};

/**
 * CSTTorrentManager
 *
 * The central class of the SymTorrent engine. Singleton.
 */
class CSTTorrentManager : 	public CBase, 
							public MSTPreferencesObserver, 
							public MKiNetworkManagerObserver,
							public MKiSocketAccepter
{
public: // constructors

	IMPORT_C static CSTTorrentManager* NewL();
	
	IMPORT_C static CSTTorrentManager* NewLC();	
	
public:
	
	/**
	 * If the local connection conencts to a master-slave network (Bluetooth piconet)
	 * then this is the type of the peer
	 */
	enum TLocalConnectionMasterSlaveType
	{
		EUndefined =0,
		EMaster,
		ESlave		
	};
	
	/**
	 * Initializes the objects needed for GridTorrent.
	 * 
	 * If the method is not called, object used only by GridTorrent are not created, thus
	 * some memory is saved.
	 */
	IMPORT_C void InitializeGridTorrentEngineL();
	
	IMPORT_C void LoadSavedTorrentsL();

	IMPORT_C ~CSTTorrentManager();
	
	/**
	 * Registers a new .torrent file and adds it to the
	 * download queue.
	 *
	 * @return KErrNone on success
	 *         KErrCopyFailed if copying the torrent file to the applications torrent-directoy fails
	 *		   KErrParsingFailed if parsing the loaded torrent file fails (bad file format?)
	 *		   KErrAlreadyOpened if the torrent file is already opened
	 *		   KErrTooLargeFile if the torrent has at least one file larger than 4GB
	 *		   KErrGeneral otherwise (failed to open the torrent file for example)
	 */
	IMPORT_C TInt OpenTorrentL(const RFile& aFile);
	IMPORT_C TInt OpenTorrentL(const TDesC& aFileName);
	IMPORT_C TInt OpenTorrentL(const TDesC& aFileName, CSTTorrent*& aTorrent);
	
	/**
	 * Opens a saved torrent from the given storrent file
	 */
	IMPORT_C TInt OpenSavedTorrentL(const TDesC& aFileName);	
	IMPORT_C TInt OpenSavedTorrentL(const TDesC& aFileName, CSTTorrent*& aTorrent);
	
	IMPORT_C void OpenTorrentDelayedL(const TDesC& aFileName);

	/**
	 * Removes a torrent form the transfer queue
	 *
	 * @param aTorrent pointer to the torrent to be closed
	 */
	IMPORT_C void RemoveTorrentL(CSTTorrent* aTorrent, TBool aDeleteIncompleteFiles = EFalse);

	/**
	 * Removes a torrent form the transfer queue
	 *
	 * @param aTorrentIndex index of the torrent to be closed
	 */
	IMPORT_C void RemoveTorrentL(TInt aTorrentIndex, TBool aDeleteIncompleteFiles = EFalse);	
	
	IMPORT_C void SaveTorrentsStateL();
	
	inline CSTPreferences* Preferences() const;		
	
	/**
	 * @return a file server session handle
	 */
	inline RFs& Fs();
	
	/**
	 * @return the 20 byte peer id.
	 */
	inline const TDesC8& PeerId() const;

	inline const TUint Key() const;		

	inline CSTTorrent* Torrent(TInt aIndex);
		
	inline CNetworkManager* NetworkManager();
	
	inline TInt TorrentCount() const;

#ifdef USE_DHT
	inline NKademlia::CBitTorrentDHT* DHT() const;
#endif
	
	/**
	 * Searches the list of torrents for a matching infohash value then attaches
	 * the passed peer connection (and it's referred CSTPeer object) to the torrent.
	 *
	 * @return KErrNone if the operation succeeds and something else otherwise ;)
	 */
	IMPORT_C TInt AttachPeerToTorrentL(const TDesC8& aInfoHash, const TDesC8& aPeerId, CSTPeerConnection* aPeerConnection);
	
	/**
	 * Registers the incoming connection and creates a new peer.
	 */
	IMPORT_C void RegisterIncomingConnectionL(RSocket* aSocket, TBool aIsLocal);
	
	IMPORT_C void SetEngineStateObserverL(MSTEngineStateObserver* aObserver);
	
	inline void RemoveEngineStateObserver();
	
	IMPORT_C void NotifyEngineStateObserverL();
	
	inline void IncIncomingConnectionCountL();
	
	inline void DecIncomingConnectionCountL();
	
	inline void IncBytesUploadedL(TInt aBytesUploaded);
	
	inline void IncBytesDownloadedL(TInt aBytesDownloaded);
	
	IMPORT_C void SetAllTorrentsFailedL();
	
	/**
	 * Closes all torrents but doesn't remove them from the saved preferences
	 * (they will be available after restarting the program).
	 *
	 * Asynchronous operation, the observer is called when the torrents are closed.
	 */
	IMPORT_C void CloseAllTorrentsL(MSTCloseTorrentObserver* aObserver);
	
	inline void SetLocalCooperationEnabled(TBool aLocalConn);
	
	inline TBool IsLocalCooperationEnabled() const;
	
	/**
	 * Indicates that the local connection operates in master-slave mode (Bluetooth)
	 */
	inline TBool IsLocalConnMasterSlaveMode() const;
	
	inline TLocalConnectionMasterSlaveType LocalConnectionMSType() const;
	
	inline void SetLocalConnectionMSType(TLocalConnectionMasterSlaveType aType);
	
	inline void SetEngineEventObserver(MSTEngineEventObserver* aObserver);
	
	inline MSTEngineEventObserver* EngineEventObserver(); 
	
	inline void SetEndGameDisabled(TBool aDisableEndgame);
	
	inline TBool IsEndGameDisabled() const;
	
public:
	
	/**
	 * Disconnects the network interface if there are no active torrents
	 */
	void CloseNetworkIfNoTorrentsActiveL();
	
	/*
	 * Called when a new UDP datagram is received from the local network (GridTorren EXT)
	 */
	void OnLocalUDPReceiveL(TInetAddr aSender, const TDesC8& aData);
	
//private:

	//void NetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection);
	
public: // observer handling

	/**
	 * Registers an observer for the events specified by iEventFlags 
	 * (use the flags specified by TSTTorrentObserverEventFlags)
	 *
	 * If aTorrent == NULL then events generated by all torrents are reported.
	 */
	IMPORT_C void AddTorrentObserverL(	MSTTorrentObserver* aObserver, 
										CSTTorrent* aTorrent = NULL,
										TUint32 iEventFlags = KSTAllTorrentObserverEvents);
	
	IMPORT_C void RemoveTorrentObserver(MSTTorrentObserver* aObserver);
	
	/**
	 * Used for notifying the registered observers
	 */
	void NotifyTorrentObserverL(CSTTorrent* aTorrent, TUint32 iEventFlags);

/*	inline MSTTorrentManagerObserver* TorrentManagerObserver() const;
	
	inline void RemoveTorrentManagerObserver();
	
	IMPORT_C void SetTorrentManagerObserverL(MSTTorrentManagerObserver* aObserver);
	
	IMPORT_C void NotifyObserverL(CSTTorrent* aTorrent, TSTTorrentStatus aEvent = ENotSpecified);*/
	
private: // from MKiSocketAccepter
	
	TBool AcceptSocketL(RSocket* aSocket, TUint aPort, CNetworkConnection& aConnection, TInt aConnIndex);
	
private: // from MKiNetworkManagerObserver
	
	virtual void NetworkManagerEventReceivedL(TKiNetworkManagerEvent aEvent);
	
	virtual void OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection, TInt aConnectionIndex);			
		
	virtual void OnNetworkConnectionDownL(CNetworkConnection& aConnection, TInt aConnectionIndex);

private: // from CSTPreferencesObserver

	virtual void SettingChangedL(TSymTorrentSetting aSetting);
	
private: // constructors
	
	CSTTorrentManager();
	
	void ConstructL();
	
private:

	/**
	 * Static wrapper for OnTimerL
	 */
	static TInt StaticOnTimerL(TAny* aObject);

	/**
	 * Called by timer periodically
	 */
	void OnTimerL();
	
	static TInt StaticOnCloseTimerL(TAny* aObject);
	
	void OnCloseTimerL();
	
	void GeneratePeerIdL();
	
	TInt OpenTorrent2L(CSTTorrent* aTorrent, TBool aStart = ETrue);
	
private:

	CNetworkManager* iNetworkManager;

	/**
	 * A periodic timer. Calls OnTimerL() periodically (started during
	 * construction)
	 */
	CPeriodic*				iTimer;
	
	CPeriodic*				iCloseTimer;
	
	/**
	 * Counter, increased by the timer in every second.
	 */
	TInt					iEllapsedTime;
	
	RPointerArray<CSTTorrent> iTorrents;
	
	CSTPreferences* iPreferences;
	
	CKiLogger* iLog;

#ifdef USE_DHT
	CKiLogger* iDHTLog;
#endif
	
	RFs iFs;
	
	/**
	 * 20 byte peer id
	 */
	TBuf8<20> iPeerId;
	
	/**
	 * Seed for random numbers
	 */
	TInt64 iSeed;

	/**
	 * A random key for every session, used by tracker requests
	 */
	TUint iKey;
	
	/**
	 * Timeout counter for closing torrents
	 */
	TInt iCloseTimeoutCounter;

//	MSTTorrentManagerObserver* iTorrentManagerObserver;
	
	CDesCArrayFlat* iDelayedTorrents;
	
	RPointerArray<CSTPeer>		iIncomingPeers;
	
	MSTEngineStateObserver*		iEngineStateObserver;
	
	TInt iIncomingConnectionCount;
	
	TInt iBytesUploaded;
	
	TInt iBytesDownloaded;
	
	MSTCloseTorrentObserver* iCloseTorrentObserver;
	
	RArray<TSTTorrentObserver>	iTorrentObservers;
	
	TBool iLocalCooperationEnabled;
	
	/**
	 * Indicates that the local connection operates in master-slave mode (Bluetooth piconet)
	 */
	TBool iLocalConnMasterSlaveMode;
	
	/**
	 * Determines whether the peer operates in master or slave mode if the local connection
	 * is master-slave based (Bluetooth piconet)
	 */
	TLocalConnectionMasterSlaveType iLocalConnectionType;
	
	MSTEngineEventObserver* iEngineEventObserver;
	
	TBool iEndgameDisabled;
	
#ifdef USE_DHT
	NKademlia::CBitTorrentDHT*	iDHT;
#endif
};

// INLINE FUNCTION IMPLEMENTATIONS

inline CSTPreferences* CSTTorrentManager::Preferences() const {
	return iPreferences;	
}

inline RFs& CSTTorrentManager::Fs() {
	return iFs;	
}

inline const TDesC8& CSTTorrentManager::PeerId() const {
	return iPeerId;	
}

inline const TUint CSTTorrentManager::Key() const {
	return iKey;
}

inline CSTTorrent* CSTTorrentManager::Torrent(TInt aIndex) {
	return iTorrents[aIndex];
}

/*inline MSTTorrentManagerObserver* CSTTorrentManager::TorrentManagerObserver() const {
	return iTorrentManagerObserver;
}
	
inline void CSTTorrentManager::RemoveTorrentManagerObserver() {
	SetTorrentManagerObserverL(0);
}*/

inline CNetworkManager* CSTTorrentManager::NetworkManager() {
	return iNetworkManager;
}

inline TInt CSTTorrentManager::TorrentCount() const {
	return iTorrents.Count();
}

inline void CSTTorrentManager::RemoveEngineStateObserver() {
	iEngineStateObserver = NULL;
}

inline void CSTTorrentManager::IncIncomingConnectionCountL() {
	iIncomingConnectionCount += 1;
	NotifyEngineStateObserverL();
}
	
inline void CSTTorrentManager::DecIncomingConnectionCountL() {
	iIncomingConnectionCount -= 1;
	NotifyEngineStateObserverL();
}
	
inline void CSTTorrentManager::IncBytesUploadedL(TInt aBytesUploaded) {
	iBytesUploaded += aBytesUploaded;
	NotifyEngineStateObserverL();
}
	
inline void CSTTorrentManager::IncBytesDownloadedL(TInt aBytesDownloaded) {
	iBytesDownloaded += aBytesDownloaded;
	NotifyEngineStateObserverL();
}

inline void CSTTorrentManager::SetLocalCooperationEnabled(TBool aLocalConn) {
	iLocalCooperationEnabled = aLocalConn;
}

inline TBool CSTTorrentManager::IsLocalCooperationEnabled() const {
	return iLocalCooperationEnabled;
}

inline TBool CSTTorrentManager::IsLocalConnMasterSlaveMode() const {
	return iLocalConnMasterSlaveMode;
}

inline CSTTorrentManager::TLocalConnectionMasterSlaveType CSTTorrentManager::LocalConnectionMSType() const {
	return iLocalConnectionType;
}

inline void CSTTorrentManager::SetLocalConnectionMSType(TLocalConnectionMasterSlaveType aType) {
	iLocalConnectionType = aType;
}

inline void CSTTorrentManager::SetEngineEventObserver(MSTEngineEventObserver* aObserver) {
	iEngineEventObserver = aObserver;
}

inline MSTEngineEventObserver* CSTTorrentManager::EngineEventObserver() {
	return iEngineEventObserver;
}

inline void CSTTorrentManager::SetEndGameDisabled(TBool aDisableEndgame) {
	iEndgameDisabled = aDisableEndgame;
}

inline TBool CSTTorrentManager::IsEndGameDisabled() const {
	return iEndgameDisabled;
}

#ifdef USE_DHT
inline NKademlia::CBitTorrentDHT* CSTTorrentManager::DHT() const {
	return iDHT;
}
#endif

#endif
