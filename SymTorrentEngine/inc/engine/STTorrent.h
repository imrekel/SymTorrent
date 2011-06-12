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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theí
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/*
 * ============================================================================
 *  Classes  : CSTTorrent
 *			  
 *  Part of  : SymTorrent
 *  Created  : 07.02.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STTORRENT_H
#define SYMTORRENT_STTORRENT_H

// INCLUDES
#include <e32base.h>
#include "STTorrentManager.h"
#include "NetworkManager.h"
#include "STTrackerConnection.h"
#include "STDefs.h"
#include "STEnums.h"
#include "STHashChecker.h"
#ifdef USE_DHT
#include "BitTorrentDHT.h"
#endif
#include "STPeerArray.h"
#include "MeasurementLog.h"
#include "TrackerManager.h"

// FORWARD DECLARATIONS
class CSTTorrentManager;
class CSTBencode;
class CSTFile;
class CSTTrackerConnection;
class CSTPeer;
class CSTPiece;
class CSTBitField;
class TInetAddr;
class CSTFileManager;
class CSTPreferences;
class CSTAnnounceList;
class CKiLogger;
class CSTPieceAccess;
class CMeasurementLog;
class CFailedPieceCollector;

enum TClosingState
{
	ETorrentNotClosing = 0,
	ETorrentClosing,
	ETorrentClosed
};

// CONSTS

//#ifdef __WINS__
// const TInt KMaxPeerConnectionCount = 1; // maximum number of simultaneous connections (+ incoming slots)
//#else
 const TInt KMaxPeerConnectionCount = 25;
//#endif

const TInt KIncomingConnectionSlots = 2; // number of slots reserved for incomming connections

/**
 * CSTTorrent
 */
class CSTTorrent : 	public CBase, 
					public MSTProxyConnectionObserver, 
					public MSTHashCheckerObserver,
					public MTrackerManagerObserver
					#ifdef USE_DHT
					,public NKademlia::MDHTGetValueObserver
					#endif
{
public: // exported methods

	/**
	 * Starts / resumes downloading and hosting of the torrent
	 */
	IMPORT_C void StartL();

	/**
	 * Stops (pauses) the transfer of this torrent
	 */
	IMPORT_C void StopL();
	
	/**
	 * Stops transferring the torrent and marks it as removable (the engine will remove it after
	 * it has been stopped)
	 */
	IMPORT_C void CloseL(TBool aDeleteIncompleteFiles = EFalse);

	/**
	 * Loads the specified .torrent file. Should be called immediately
	 * after construction.
	 *
	 * @return KErrNone on success
	 */
	IMPORT_C TInt LoadL(const TDesC& aTorrentFileName);
	IMPORT_C TInt LoadL(const RFile& aTorrentFile);
	
	/**
	 * Opens a saved torrent
	 */
	IMPORT_C TInt LoadSTorrentL(const TDesC& aSTorrentFileName);
	
	/**
	 * Checks whether there is enough disk space for downloading the torrent
	 */
	IMPORT_C TBool HasEnoughDiskSpaceToDownload();
			
	/**
	 * Searches the torrent's peer list for a peer with the given ip address/port.
	 *
	 * @return the peer with the passed ip address and port, NULL if it the search is unsuccessfull.
	 */		
	IMPORT_C CSTPeer* GetPeer(const TSockAddr& aAddress);
	
	/**
	 * @return peer at the given index
	 */
	inline CSTPeer* Peer(TInt aIndex);
	
	/**
	 * Searches the torrent's peer list for a peer with the given peer ID
	 * 
	 * @return the peer with the given ID or NULL if no peer is found
	 */
	CSTPeer* GetPeer(const TDesC8& aPeerId);
	
	/**
	 * Adds a peer to the torrent's peer list. 
	 *
	 * @return if a peer with the same ip address is already in 
	 * the list, the function returns KErrAlreadyExists. KErrNone otherwise.	 
	 */
	IMPORT_C TInt AddPeerL(CSTPeer* aPeer);
	
	/**
	 * Tries to add a peer to the torrent's peer list. If a peer with the same
	 * address is already in the list or the maximum number of peers reached,
	 * then the request is discarded.
	 */
	IMPORT_C TInt AddPeerL(const TInetAddr& aAddress);
	
	/**
	 * Adds a local peer to the torrent. If a peer with the same
	 * address is already in the list or the maximum number of local peers reached,
	 * then the request is discarded.
	 */
	IMPORT_C TInt AddLocalPeerL(const TSockAddr& aAddr);
	
	/**
	 * Adds a push peer to the torrent's peer list.
	 * Push peers are connected by the engine even if no pieces are needed from them.
	 */
	IMPORT_C void CSTTorrent::AddPushPeerL(TInetAddr aAddress);
	
	/**
	 * Saves the sate of the torrent
	 */
	IMPORT_C void SaveStateL();
	
	/**
	 * Removes a peer
	 */
	IMPORT_C void DeletePeer(const TSockAddr& aAddress);
	
	/**
	 * Sets the torrent to failed state 
	 * (for instance in case it has run out of peers and failed to connect to the tracker)
	 */
	IMPORT_C void SetFailedL();
	
	/**
	 * Asynchronously checks the hash values of the files of the torrent.
	 *
	 * Updates the given progress dialog periodically and closes it
	 * when the process has finished.
	 */
	IMPORT_C void CheckHashL(CAknProgressDialog* aProgressDialog);
	
	/**
	 * Sets whether a file is skipped (not downloaded on purpose).
	 */
	IMPORT_C void SetFileSkippedL(TInt aFileIndex, TBool aSkipped);
	
	/**
	 * Marks/unmarks all files skipped
	 */
	IMPORT_C void SetAllFilesSkippedL(TBool aSkipped);
	
	/**
	 * Set files that are larger than 4GB skipped.
	 * 
	 * @return the number of skipped files
	 */
	IMPORT_C TInt SkipTooLargeFiles();
	
#ifdef USE_DHT
	void DHTAnnounceL();
#endif

public: //inlines
	
	/**
	 * Total number of bytes downloaded since the first connection to the tracker.
	 */
	inline TInt64 BytesDownloaded() const;
	
	/**
	 * Total number of bytes uploaded since the first connection to the tracker.
	 */
	inline TInt64 BytesUploaded() const;
	
	/**
	 * The number of bytes the client still has to download.
	 */
	inline TInt64 BytesLeft() const;
	
	/**
	 * @return the info hash of the torrent.
	 */
	inline const TDesC8& InfoHash() const;
	
	inline const CSTBitField* BitField() const;
	
	inline const CSTBitField* ToDownloadBitField() const;
	
	inline const CSTBitField* LocalDownloadBitField() const;
	
	inline TInt PieceLength() const;
			
	inline TInt IndexOfPiece(const CSTPiece* aPiece) const;
	
	inline CSTPiece* Piece(TInt aIndex);

	/**
	 * @return the download path (the torrent's parent directory), ends with \ (backslash)
	 */
	inline const TDesC& Path() const;

	/**
	 * @return the name of the torrent (the filename for a single-file torrent and
	 * the name of the parent directory for a multi-file torrent)
	 */
	inline const TDesC& Name() const;

	inline const TDesC8& Comment() const;
	
	inline const TDesC8& CreatedBy() const;

	/**
	 * @return the total size of the torrent in bytes
	 */
	inline TInt64 Size() const;
	
	/**
	 * Size of the downloadable part of the torrent in bytes
	 */
	inline TInt64 ToDownloadSize() const;

	inline CSTFileManager& FileManager() const;

	inline TBool IsComplete() const;

	/**
	 * @return the number of active peer connections using the primary
	 * network interface
	 */
	inline TInt ConnectionCount() const;
	
	/**
	 * @return the number of active peer connections using the local
	 * network interface
	 */
	inline TInt LocalConnectionCount() const;

	inline TReal DownloadPercent() const;
	
	/**
	 * Download speed in bytes per second
	 */
	inline TReal DownloadSpeed() const;
	
	/**
	 * Upload speed in bytes per second
	 */
	inline TReal UploadSpeed() const;
	
	inline TSTTorrentStatusInfo StatusInfo() const;
	
	inline TInt PieceCount() const;
	
	inline TInt FileCount() const;
	
	inline const CSTFile* File(TInt aIndex);
	
	inline CSTTorrentManager* TorrentMgr() const;
	
	inline CSTPreferences* Preferences();
		
	inline const TDesC& SavedTorrent() const;
		
	inline TBool IsActive() const;
	
	inline const TDesC& FailReason() const;
	
	inline TBool IsFailed() const;
	
	inline CKiLogger* Log() const;
		
	/**
	 * Indicates that the torrent has entered end game mode
	 */
	inline TBool EndGame() const;
		
	/**
	 * True if the torrent is stopped (paused) and has already announced to the tracker 
	 * with a "Stopped" event.
	 */
	inline TBool IsClosed();
	
//	inline TInt ActiveConnectionCount() const;
	
	/**
	 * @return the time of the last successfull tracker connection (announce)
	 */
	inline TTime LastTrackerConnectionTime() const;
	
	inline TInt TrackerRequestInterval() const;
	
	inline TInt PeerCount() const;
	
	/**
	 * The number of peers on the local (secondary) network interface
	 */
	inline TInt LocalPeerCount() const;

	/**
	 * The number of peers on the primary network interface
	 */
	inline TInt PrimaryPeerCount() const;
	
	/**
	 * Checks whether the torrent has an active tracker connection
	 */
//	inline TBool IsConnectingToTracker() const;
	
	inline TInt EllapsedTime() const;
	
//	inline CSTPieceAccess* PieceAccess();
	
	inline CTrackerManager& TrackerManager() { return *iTrackerManager; }

public:

	CSTTorrent(CSTTorrentManager* aTorrentMgr);
	
	void ConstructL();
	
	~CSTTorrent();
	
	/**
	 * Registers the completation of a piece
	 */
	void PieceDownloadedL(CSTPiece* aPiece, CSTPeer* aPeer, TBool aNotifyTorrentObserver = ETrue);
	void PieceDownloadedL(CSTPiece* aPiece, TBool aNotifyTorrentObserver = ETrue);
	
	/**
	 * Called when the hash value of a downloaded piece is incorrect
	 */
	void PieceHashFailedL(CSTPiece* aPiece);

	void UpdateBytesDownloadedL(TInt aBytesDownloaded, TBool aNotifyObserver = ETrue);
	
	/**
	 * Sets the torrent to an undownloaded state (as if it had just been started)
	 */
	void ResetDownloadsL(TBool aNotifyObserver);
	
	void UpdateBytesUploadedL(TInt aBytesUploaded, TBool aNotifyObserver = ETrue);
		
	/**
	 * Called when a peer is disconnected
	 */
	void PeerDisconnectedL(CSTPeer* aPeer, TBool iPeerWireConnected);
	
	/**
	 * @return a piece which is
	 */
	CSTPiece* GetPieceToDownloadL(CSTPeer* aPeer);
	
	/**
	 * Removes a piece from the list of pieces which are
	 * being downloaded.
	 */
	void RemovePieceFromDownloading(CSTPiece* aPiece);
	
	/**
	 * Called by the tracker to indicate failure
	 */
	void TrackerFailedL();
	
	/**
	 * Increases the number of connected primary peers (PW for Peer Wire,
	 * it indicates that these peers are done with handshaking and
	 * exchanging PW messages)
	 */
	void IncreaseConnectionCountL();
	
	/**
	 * Increases the number of connected local peers (PW for Peer Wire,
	 * it indicates that these peers are done with handshaking and
	 * exchanging PW messages)
	 */
	void IncreaseLocalConnectionCountL();
	
	TBool HasTimeoutlessPeer();
	
	void IncreaseNumberOfPeersHavePiece(TInt aPieceIndex);
	
	void DecreaseNumberOfPeersHavingPiece(TInt aPieceIndex);
	
	/**
	 * Called by the peer connections when a new PIECE message is received in End Game mode.
	 *
	 * @param aPiece the received piece
	 * @param aPeer the peer which received the piece
	 */
	void EndGamePieceReceivedL(CSTPiece* aPiece, CSTPeer* aPeer);
	
	/**
	 * Disconnects and deletes any primary peer with the given ID. Used when a local
	 * peer is added with the smae ID (local peers are preferred to primary peers)
	 */
	void DeletePrimaryPeerL(const TDesC8& aPeerId);
	
	/**
	 * Sets the local network bitfield according to the given bitfield (does a bitwise or)
	 */
	void HaveLocalNetPiecesL(const TDesC8& aBitFieldDes, const CSTPeer& aPeer);
	
	void BroadcastLocalPeerL(TUint32 aAddress, TUint aPort, const CSTPeer& aException);
	
	/**
	 * Sets a bit of the local net bitfield
	 */
	void HaveLocalNetPieceL(TInt aPieceIndex, const CSTPeer& aPeer);
	
	/**
	 * Fills the passed array with the pointers to the local peers
	 */
	void GetLocalPeersL(RPointerArray<CSTPeer>& aPeers);		
	
	/**
	 * Reports a critical fault. Displays and error popup pauses the torrent.
	 */
	void ReportCriticalFaultL(TSTCriticalFaultType aFault);
	
	inline CMeasurementLog* MeasurementLog();

#ifdef USE_DHT
private: // from MDHTGetValueObserver

	void ValueFoundL(const NKademlia::CKey& aKey, const TBinDataPtr& aValue);
	/**
	 * Called by the framework when the get value request for the given key is over
	 */
	void GetValueFinishedL(const NKademlia::CKey& aKey);
#endif
	
private: // from MSTHashCheckerObserver

	void HashCheckFinishedL(CSTHashChecker* aHashChecker);

private: // from MSTProxyConnectionObserver

	void ReportProxyConnectionL(TBool aProxyConnectionSucceeded);
	
private: // from MTrackerManagerObserver
	
	void AnnouncedToAllTrackersL(TTrackerConnectionEvent aEvent);
	
private:

	/**
	 * Adds a file to the torrent's file list
	 */
	void AddFileL(const TDesC& aRelativePath, TInt64 aSize);
	
	void AddFileL(const TDesC8& aRelativePath, TInt64 aSize);

	/**
	 * Reads the content of the .torrent from the parsed bencoded object.
	 */
	TInt ReadFromBencodedDataL(CSTBencode* aBencodedTorrent, const TDesC& aPath = KNullDesC);

	void CalculateFileFragmentsL();
	
	/**
	 * Called by the timer (from CSTTorrentManager's OnTimerL()) in every second
	 */
	void OnTimerL();

	void SetStatusInfoL(TSTTorrentStatusInfo aStatus);
	
//	void SetComplete();
	
	/**
	 * Enables end game mode.	 
	 */
	void EnableEndGameL();
	
	/**
	 * Disables end game mode
	 */
	void DisableEndGameL();
	
	/**
	 * Retuns whether the conditions to enter endgame is met or not
	 */
	TBool IsEndGameConditionsMet();
	
	/**
	 * (Re)calculates iBytesToDownload from the iToDownloadBitField
	 */
	void CalculateBytesToDownload();
	
	/**
	 * Checks the bitifleds of the torrent to determine whether the download is
	 * complete. Changes iComplete accordingly.
	 */
	void CheckDownloadCompleteL();
	
private:

	HBufC* iStatusText;

	/**
	 * True if the object is consistent (contains the parsed data of a valid .torrent
	 * file).
	 */
	TBool iValid;

	/**
	 * Name of the torrent (the parent directory in a multi-file torrent or
	 * the filename in a single-file torrent).
	 */
	HBufC* iName;
	 
	/**
	 * Download path + parent directory
	 */
	HBufC* iPath;
	
	/**
	 * Tracker announce URL
	 */
//	HBufC8* iAnnounce;

//	CSTAnnounceList* iAnnounceList;
	
	/**
	 * Comment (optional)
	 */
	HBufC8* iComment;
	
	/**
	 * Creator name (optional)
	 */
	HBufC8* iCreatedBy;
	
	/**
	 * Date of creation (option)
	 */
	TTime iCreationDate;
	
	/**
	 * Length of a piece
	 */
	TInt iPieceLength;
	
	/**
	 * Pointer to the singleton torrent manager
	 */
	CSTTorrentManager* iTorrentMgr;
	
	/**
	 * The files of the torrent
	 */
	RPointerArray<CSTFile> iFiles;
	
	/**
	 * The pieces of the torrent
	 */
	RPointerArray<CSTPiece> iPieces;
	
	TBuf8<20> iInfoHash;
	
	/**
	 * Total number of bytes uploaded
	 */
	TInt64 iBytesUploaded;
	
	/**
	 * The total number of bytes downloaded
	 */
	TInt64 iTotalBytesDownloaded;
	
	/**
	 * Total number of bytes left from the torrent
	 */
	TInt64 iTotalBytesLeft;
	
	/**
	 * Number of bytes left to download
	 */
	TInt64 iDownloadBytesLeft;
	
	/**
	 * Size of the downloadable content (can be less than the size of the torrent
	 * if some files/pieces are skipped)
	 */
	TInt64 iBytesToDownload;
	
	RSTPeerArray iPeers;	
	
	RPointerArray<CSTPeer> iDisconnectedPeers;		
	
	TInt iEllapsedTime;
	
	CSTBitField* iBitField;
	
	/**
	 * Bitfield that marks the pieces to download
	 */
	CSTBitField* iToDownloadBitField;
	
	/**
	 * Pieces that has been received from local peers
	 */
	CSTBitField* iLocalDownloadBitField;
	
	/**
	 * Pieces that are in the local cluster
	 */
	CSTBitField* iLocalNetBitField;
	
	/**
	 * True if the torrent is being downloaded.
	 * False if the torrent is inactive (waiting in queue).
	 */
	TBool iActive;
	
	/** 
	 * The number of consecutive failed tracker connection attempts.
	 */
	TInt iTrackerFailures;
	
	RPointerArray<CSTPiece> iDownloadingPieces;

	/**
	 * Manages the files of the torrent (creating, writing, reading)
	 */
	CSTFileManager* iFileManager;

	/**
	 * True if the torrent is complete (finished downloading)
	 */
	TBool iComplete;

	/**
	 * The number of complete pieces
	 */
	TInt iDownloadedPieceCount;
	
	TInt iPiecesLeftToDownloadCount;

	TInt iConnectionCount;
	
	TInt iLocalConnectionCount;

	TReal iDownloadPercent;

	/**
	 * Bytes downloaded in the current second
	 */
	TInt iBytesPerSecond;

	TReal iAvarageBytesPerSecond;
	
	/**
	 * Bytes uploaded in the current second
	 */
	TInt iUploadedBytesPerSecond;

	TReal iAvarageUploadedBytesPerSecond;

	TSTTorrentStatusInfo iStatusInfo;
	
	HBufC* iSavedTorrent;
	
	TTrackerConnectionEvent iLastTrackerEvent;
	
	TBool iFailed;
	
	HBufC* iFailReason;
	
	TClosingState iClosingState;
	
//	TInt iActiveConnectionCount;
	
	TInt64 iRandomSeed;
	
	/**
	 * Indicates that the torrent has entered end game mode
	 */
	TBool iEndGame;
	
	/**
	 * Number of seconds for which the status info is displayed (tracker connected / tracker failed)
	 */
	TInt iStatusInfoDelay;
	
	TTime iLastTrackerConnectionTime;
	
	//RPointerArray<CSTPiece> iEndGamePiecesInTransit;
	
	CSTHashChecker* iHashChecker;
	
	CKiLogger* iLog;
	
	// GridTorrent energy measurements
	
	CMeasurementLog* iMeasurementLog;
	
	CTrackerManager* iTrackerManager;
	
public:
	
	TInt iSentLocalRequests; // number of UDP requests sent
	
	TInt iLocalSubPiecesForRequests; // number of pieces received as answer for an outgoing UDP request
	
	TInt iLocalSubPiecesNotRequested; // number of local pieces received but not requested
	
	TInt iLocalSubPiecesReceivedAlreadyDownloaded;
	
	TInt iLocalSubPiecesReceivedNotMatchPieceBeginning;
	
	TInt iLocalRequestTimeouts;
	
	TInt64 iLocalSubPiecesForRequestsSize;
	TInt64 iLocalSubPiecesNotRequestedSize;
	TInt64 iLocalSubPiecesReceivedAlreadyDownloadedSize;
	
	TInt64 iLocalBytesUploaded;
	
	TInt64 iLocalBytesDownloaded;
	
	TInt64 iLongRangeBytesUploaded;
	
	TInt64 iLongRangeBytesDownloaded;
	
	TInt iLongRangeSubPiecesReceived;
	TInt64 iLongRangeSubPiecesReceivedSize;
	
//	CSTPieceAccess* iPieceAccess;
	
#ifdef USE_DHT	
	TBool iDHTAnnouncing;
	
	TInt iPeersFromDHTCount;
#endif
	
	friend class CSTTorrentManager;
	
	friend class CSTHashChecker;
};


// INLINE FUNCTION IMPLEMENTATIONS

inline const TDesC8& CSTTorrent::InfoHash() const {
	return iInfoHash;	
}

inline TInt64 CSTTorrent::BytesDownloaded() const {
	return iTotalBytesDownloaded;
}

inline TInt64 CSTTorrent::BytesUploaded() const {
	return iBytesUploaded; 
}

inline TInt64 CSTTorrent::BytesLeft() const {
	return iTotalBytesLeft;
}

inline const CSTBitField* CSTTorrent::BitField() const {
	return iBitField;	
}

inline TInt CSTTorrent::PieceLength() const {
	return iPieceLength;	
}

inline TInt CSTTorrent::IndexOfPiece(const CSTPiece* aPiece) const {
	return iPieces.Find(aPiece);	
}

inline const TDesC& CSTTorrent::Path() const {
	return *iPath;
}

inline const TDesC& CSTTorrent::Name() const {
	return *iName;
}

inline const TDesC8& CSTTorrent::Comment() const {
	if (iComment)
		return *iComment;
	return KNullDesC8;
}

inline const TDesC8& CSTTorrent::CreatedBy() const {
	if (iCreatedBy)
		return *iCreatedBy;
	return KNullDesC8;
}

inline TInt64 CSTTorrent::Size() const {
	return iTotalBytesDownloaded + iTotalBytesLeft;
}

inline CSTFileManager& CSTTorrent::FileManager() const {
	return *iFileManager;
}

inline TBool CSTTorrent::IsComplete() const {
	return iComplete;
}

inline TInt CSTTorrent::ConnectionCount() const {
	return iConnectionCount;
}

inline TInt CSTTorrent::LocalConnectionCount() const {
	return iLocalConnectionCount;
}

//inline void CSTTorrent::NotifyTorrentManagerL() {
//	iTorrentMgr->NotifyObserverL(this);
//}

inline TReal CSTTorrent::DownloadPercent() const {
	return iDownloadPercent;
}

inline TReal CSTTorrent::DownloadSpeed() const {
	return iAvarageBytesPerSecond;
}

inline TReal CSTTorrent::UploadSpeed() const {
	return iAvarageUploadedBytesPerSecond;
}

inline TSTTorrentStatusInfo CSTTorrent::StatusInfo() const {
	return iStatusInfo;
}

inline TInt CSTTorrent::PieceCount() const {
	return iPieces.Count();
}

inline CSTPiece* CSTTorrent::Piece(TInt aIndex) {
	return iPieces[aIndex];
}

inline TInt CSTTorrent::FileCount() const {
	return iFiles.Count();
}
	
inline const CSTFile* CSTTorrent::File(TInt aIndex) {
	return iFiles[aIndex];
}

inline CSTTorrentManager* CSTTorrent::TorrentMgr() const {
	return iTorrentMgr;
}

inline const TDesC& CSTTorrent::SavedTorrent() const {
	if (iSavedTorrent)
		return *iSavedTorrent;
	
	return KNullDesC;
}

inline TBool CSTTorrent::IsActive() const {
	return iActive;
}

inline const TDesC& CSTTorrent::FailReason() const {
	if (iFailReason)
		return *iFailReason;
	
	return KNullDesC;
}

inline TBool CSTTorrent::IsFailed() const {
	return iFailed;
}

inline TBool CSTTorrent::IsClosed() {
	return (iClosingState == ETorrentClosed);
}

//inline TInt CSTTorrent::ActiveConnectionCount() const {
//	return iActiveConnectionCount;
//}

inline CSTPreferences* CSTTorrent::Preferences() {
	return iTorrentMgr->Preferences();
}

inline TTime CSTTorrent::LastTrackerConnectionTime() const {
	return iLastTrackerConnectionTime;
}

inline TBool CSTTorrent::EndGame() const {
	return iEndGame;
}

inline TInt CSTTorrent::PeerCount() const {
	return iPeers.Count();
}

inline TInt CSTTorrent::LocalPeerCount() const {
	return iPeers.LocalPeerCount();
}

inline TInt CSTTorrent::PrimaryPeerCount() const {
	return iPeers.PrimaryPeerCount();
}

inline CKiLogger* CSTTorrent::Log() const {
	return iLog;
}

/*inline TBool CSTTorrent::IsConnectingToTracker() const {
	if (iTrackerConnection)
		return ETrue;
	else
		return EFalse;
}*/

inline TInt CSTTorrent::EllapsedTime() const {
	return iEllapsedTime;
}

inline const CSTBitField* CSTTorrent::LocalDownloadBitField() const {
	return iLocalDownloadBitField;
}

inline const CSTBitField* CSTTorrent::ToDownloadBitField() const {
	return iToDownloadBitField;
}

inline TInt64 CSTTorrent::ToDownloadSize() const {
	return iBytesToDownload;
}

inline CSTPeer* CSTTorrent::Peer(TInt aIndex) {
	return iPeers[aIndex];
}

inline CMeasurementLog* CSTTorrent::MeasurementLog() {
	return iMeasurementLog;
}

/*inline CSTPieceAccess* CSTTorrent::PieceAccess() {
	return iPieceAccess;
}*/

#endif
