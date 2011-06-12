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

/*
 * ============================================================================
 *  Name     : CSTPeerConnection from STPeerConnection.h
 *  Part of  : SymTorrent
 *  Created  : 22.02.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STPEERCONNECTION_H
#define SYMTORRENT_STPEERCONNECTION_H

// INCLUDES
#include "ConnectedSocket.h"
#include "STEnums.h"
#include "STBlockRequest.h"
#include "STDefs.h"
#include "STPieceAccess.h"

// FORWARD DECLARATIONS
class CDesC16Array;
class CSTPeer;
class CSTTorrent;
class CSTPiece;
class CSTTorrentManager;
class CSTPreferences;
class CKiLogger;

// CONSTANTS
const TUint32 KFlagAmChoking = 1; 		// ...0001
const TUint32 KFlagAmInterested = 2;	// ...0010
const TUint32 KFlagPeerChoking = 4; 	// ...0100
const TUint32 KFlagPeerInterested = 8; 	// ...1000

// BitTorrent message ids
const TInt KMessageIdChoke = 0;
const TInt KMessageIdUnchoke = 1;
const TInt KMessageIdInterested = 2;
const TInt KMessageIdNotInterested = 3;
const TInt KMessageIdHave = 4;
const TInt KMessageIdBitfield = 5;
const TInt KMessageIdRequest = 6;
const TInt KMessageIdPiece = 7;
const TInt KMessageIdCancel = 8;
// GridTorrent messages
const TInt KMessageIdLocalHave = 230;
const TInt KMessageIdLocalBitfield = 231;
const TInt KMessageIdLocalPeer = 232;

/**
 * TSTPieceToDownload
 */
class CSTPieceToDownload : public CBase
{
public:
	
	/**
	 * TPieceRequest
	 */
	class TPieceRequest
	{
	public:
		TPieceRequest(TInt aBegin, TInt aEnd, TInt aTimeOfSending)
		 : iBegin(aBegin), iEnd(aEnd), iTimeOfSending(aTimeOfSending) {}
		
		TInt Length() const { return iEnd - iBegin; }
	public:
		TInt iBegin;
		TInt iEnd;
		TInt iTimeOfSending;
	};
	
public:
	
	CSTPieceToDownload(CSTPiece* aPiece, TInt aLastRequestTime);
	
	~CSTPieceToDownload() { iPendingRequests.Reset(); }
	
	const CSTPiece* Piece() const {	return iPiece;	}

public:

	CSTPiece* iPiece;
	
	RArray<TPieceRequest> iPendingRequests;
	
	TInt iLastRequestTime;
	
	/**
	 * The size of the part of the piece that has already been requested or downloaded
	 */
	TInt iRequestedSize;
	
//	TInt iLastRequestBegin;
	
//	TInt iLastRequestLength;
};

/**
 * CSTPeerConnection
 */
class CSTPeerConnection : 	public CKiConnectedSocket, 
							public MKiConnectedSocketWriterObserver,
							public MPieceWriteObserver
{
public:

	enum TConnectionCloseOrder
	{
		EDeletePeer,
		EIncreaseErrorCounter,
		EDelayReconnect,
		ENotSpecified
	};

	
	CSTPeerConnection(CSTPeer& aPeer, CSTTorrent* aTorrent, CSTTorrentManager* aTorrentMgr);


	void ConstructL();
	
	/**
	 * Constructs the connections from an existing, already connected socket (incoming connection!)
	 */
	void ConstructL(RSocket* aSocket);
	
	void ConnectL();

	void CloseL(const TDesC8& aReason = KNullDesC8);
	
	void CloseL(TConnectionCloseOrder aOrder, const TDesC8& aReason = KNullDesC8);

	inline TPeerConnectionState State();

	~CSTPeerConnection();
	
	// virtual TUint DefaultReconnectDelay();

	void ChangeState(TPeerConnectionState aState);

	/**
	 * Called by the timer (in CCommEngine) in every second
	 */
	void OnTimerL();
	
	inline TBool IsHandled() const;
		
//	inline TUint ReconnectAfter() const;
	
	inline TInt Retries() const;
	
	const TSockAddr& RemoteAddress();
	
	void LogL(const TDesC& aText);
	
	inline CSTPiece* PieceToDownload();
	
	inline TConnectionCloseOrder CloseOrder() const;
	
	inline CSTPeer* Peer();
	
	inline void SetTorrent(CSTTorrent* aTorrent);
	
	inline const RPointerArray<CSTPieceToDownload>& PiecesToDownload() const;
		
	/**
	 * Gets the address of the connected peer
	 */
	inline void RemoteAddress(TSockAddr& aAddress) const;
	
	/**
	 * @return true if the peer is a push peer.
	 */
	TBool IsPushPeer() const;
	
	/**
	 * Sends a cancel message for the given piece if the peer has an active request for it.
	 */
	void CancelPieceRequestL(CSTPiece* aPiece);
	
	/**
	 * Cancels all piece requests.
	 */
	void CancelAllPieceRequestsL();
	
	/**
	 * Change the peer of the connection
	 */
	void SetPeer(CSTPeer* aPeer);
	
	/**
	 * Tries to handle a piece message received via the short-range link. If the piece was
	 * requested by this peer, then it is processed and true is returned.
	 */
	TBool HandleIncomingLocalPieceL(TInt aIndex, TInt aBegin, const TDesC8& aData);
	
public: // getters

	inline TBool IsChoking() const;
	
	inline TBool IsInterested() const;
	
	inline TBool IsPeerChoking() const;
	
	inline TBool IsPeerInterested() const;
	
	inline CSTTorrent& Torrent();
	
public:

	void SetChokingL(TBool aChoking);
	
	void SetInterestedL(TBool aInterested);
	
	void IssueDownloadL();
	
public: // message senders

	void SendKeepAliveMessageL();
	
	void SendBitfieldMessageL();
	
	void SendInterestedMessageL();
	
	void SendNotInterestedMessageL();
	
	//void SendRequestMessageL();
	
	void SendRequestMessageL(CSTPieceToDownload& aPiece);
	
	void SendCancelMessageL(CSTPieceToDownload& aPiece);
	
	void SendChokeMessageL();
	
	void SendUnchokeMessageL();
	
	void SendHaveMessageL(TInt aPieceIndex);
	
	void SendPieceMessageL(TInt aPieceIndex, TInt aBegin, TInt aLength);
	
	void SendLocalHaveMessageL(TInt aPieceIndex);
	
	void SendLocalBitfieldMessageL(const TDesC8& aBitField);
	
	void SendLocalPeerMessageL(TUint32 aAddress, TUint aPort);
	
private: // message handlers
	
	void HandlePieceMessageL(TInt aIndex, TInt aBegin, const TDesC8& aData);
	
private:

	inline void SetPeerChoking(TBool aChoking);
	
	void SetPeerInterestedL(TBool aInterested);	
	
	void SendHandshakeMessageL();
	
	void SendByteL(TChar aByte);
	
	/**
	 * Sends a 4 byte integer big-endian encoded
	 */
	void SendIntL(TUint32 aInteger);
	
	void PutIntToSendBufferL(TUint32 aInteger);
	
	void PutByteToSendBufferL(TChar aByte);
	
	inline void SetHandled();
	
	TUint ReadInt(TInt aIndex = 0);
	
	void IssueUploadL();

	void SetPeerWireConnectedL();
	
	CSTTorrentManager* TorrentMgr();
	
	CSTPreferences* Preferences();

private: // from CKiConnectedSocket

	void OnSocketConnectSucceededL();
	
	void OnSocketConnectFailedL(TInt aReason);

	void HandleWriteErrorL();

	void HandleReadErrorL();
	
	void OnReceiveL();
	
	void SocketOpenedL(TBool aResult, RSocket& aSocket);
	
	void OnDataSentL(TInt aSize);
	
private: // from MKiConnectedSocketWriterObserver
	
	void OnSocketWriteFinishedL();
	
private: // from MPieceWriteObserver
	
	void OnPieceWriteFailedL(const CSTPiece& aPiece, TInt aBegin, TInt aLength);
	void OnPieceWriteCompleteL(const CSTPiece& aPiece, TInt aBegin, TInt aLength);
	
private:

	TUint32					iStatusFlags;

	CSTPeer* 				iPeer;
	
	CSTTorrent* 			iTorrent;
	
	TConnectionCloseOrder   iCloseOrder;

	/**
	 * Delay before reconnecting
	 */
	TUint					iReconnectAfter;
	
	/**
	 * The number of connection retries
	 */
	TUint					iRetries;

	TBool					iHandled;
	
	TBool					iHeadersReceived;

	TPeerConnectionState	iState;
	
	/**
	 * Used to track the time spent since the last event.
	 * Increased by OnTimerL in every second.
	 */
	TInt					iEllapsedTime;
	
	TInt 					iLastMessageReceivedTime;
	
	TInt					iLastMessageSentTime;
	
	TInt 					iLastRequestTime;
	
	CSTPiece*				iPieceToDownload;
	
	TBool					iHasPendingDownloadRequest;

	/**
	 * Used to indicate that the peer has managed to build up the peer wire connection
	 */
	TBool					iPeerWireConnected;
	
	RArray<TSTBlockRequest> iIncomingRequests;
	
	TBool					iIncomingConnection;
	
	CSTTorrentManager* 		iTorrentMgr;
	
	RPointerArray<CSTPieceToDownload> iPiecesToDownload;
	
	CKiLogger*				iLog;
	
	TBool					iGridTorrentExtensionEnabled;
	
	/**
	 * Indicates that a piece is being uploaded
	 */
	TBool 					iUploadingPiece;
	
	TInt iLocalSentRequestCount;
	TInt iLocalRequestWithResponseCount;
	
	friend class CSTPeer;
};

// INLINE FUNCTION IMPLEMENTATIONS

inline TPeerConnectionState CSTPeerConnection::State() {
	return iState;
}

inline void CSTPeerConnection::SetHandled() {
	//iLog->WriteLineL(_L("Message handled!"));
	iHandled = ETrue;		
}

inline TBool CSTPeerConnection::IsHandled() const {
	return iHandled;		
}

//inline TUint CSTPeerConnection::ReconnectAfter() const {
//	return iReconnectAfter;		
//}

inline TInt CSTPeerConnection::Retries() const {
	return iRetries;		
}

inline TBool CSTPeerConnection::IsChoking() const {
	return (KFlagAmChoking & iStatusFlags);	
}

inline TBool CSTPeerConnection::IsInterested() const{
	return (KFlagAmInterested & iStatusFlags);	
}
	
inline TBool CSTPeerConnection::IsPeerChoking() const {
	return (KFlagPeerChoking & iStatusFlags);	
}
	
inline TBool CSTPeerConnection::IsPeerInterested() const {
	return (KFlagPeerInterested & iStatusFlags);	
}

inline CSTTorrent& CSTPeerConnection::Torrent() {
	return *iTorrent;	
}

inline CSTPiece* CSTPeerConnection::PieceToDownload() {
	return iPieceToDownload;	
}

inline CSTPeerConnection::TConnectionCloseOrder CSTPeerConnection::CloseOrder() const {
	return iCloseOrder;
}

inline CSTPeer* CSTPeerConnection::Peer() {
	return iPeer;
}

inline void CSTPeerConnection::SetTorrent(CSTTorrent* aTorrent) {
	iTorrent = aTorrent;
}

inline const RPointerArray<CSTPieceToDownload>& CSTPeerConnection::PiecesToDownload() const {
	return iPiecesToDownload;
}

inline void CSTPeerConnection::RemoteAddress(TSockAddr& aAddress) const {
	iSocket->RemoteName(aAddress);
}

#endif

// End of File
