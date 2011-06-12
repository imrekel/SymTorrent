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

/**
 * ============================================================================
 *  Classes  : CSTPeer
 *			  
 *  Part of  : SymTorrent
 *  Created  : 20.02.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STPEER_H
#define SYMTORRENT_STPEER_H

// INCLUDES
#include <in_sock.h>
#include <e32std.h>
#include "STPeerConnection.h"
#include "STEnums.h"
#include "STDefs.h"
#include "FailedPieceCollector.h"

// FORWARD DECLARATIONS
class CSTPeerConnection;
class CSTBitField;
class CSTTorrent;
class CSTTorrentManager;

// CONSTANTS

/**
 * CSTPeer
 */
class CSTPeer : public CBase
{
public:

	static CSTPeer* NewL(const TSockAddr& aAddress, const TDesC8& aPeerId, TInt aBitFieldLength, TBool aLocal = EFalse, TBool aPushPeer = EFalse);
	
	static CSTPeer* NewLC(const TSockAddr& aAddress, const TDesC8& aPeerId, TInt aBitFieldLength, TBool aLocal = EFalse, TBool aPushPeer = EFalse);

	static CSTPeer* NewL(const TSockAddr& aAddress,  TInt aBitFieldLength, TBool aLocal = EFalse, TBool aPushPeer = EFalse);
	
	static CSTPeer* NewLC(const TSockAddr& aAddress, TInt aBitFieldLength, TBool aLocal = EFalse, TBool aPushPeer = EFalse);
	
	static CSTPeer* NewL(RSocket* aSocket, CSTTorrentManager* aTorrentMgr, TBool aLocal = EFalse);
	
	static CSTPeer* NewLC(RSocket* aSocket, CSTTorrentManager* aTorrentMgr, TBool aLocal = EFalse);	
	
	~CSTPeer();
	
	inline TSockAddr& Address();
	
	TPeerConnectionState State() const;
	
	/**
	 * @return 20 byte peer id or KNullDesC8 if not specified
	 */
	inline const TDesC8& PeerId() const;
	
	/**
	 * Sets the peer's 20-byte-long ID
	 */
	void SetPeerIdL(const TDesC8& aPeerId);
	
	//inline CSTBitField* BitField();
		
	void HavePiece(TInt aPieceIndex, CSTTorrent& aTorrent);
	
	void HavePiecesL(const TDesC8& aBitFieldDes, CSTTorrent& aTorrent);
	
	inline const CSTBitField* BitField() const;
	
	/**
	 * Initiates a TCP/IP connection to this peer.
	 */
	void ConnectL(CSTTorrent& aTorrent, CSTTorrentManager* aTorrentMgr);
	
	void OnTimerL(CSTTorrent* aTorrent, TInt aTorrentEllapsedTime);
	
//	inline CSTPiece* PieceToDownload();

	inline const RPointerArray<CSTPieceToDownload>* PiecesToDownload() const;
	
	inline void ResetErrorCounter();
	
	inline TBool IsDeletable() const;
	
	inline TBool HadRequestTimeout() const;
	
	inline void SetHadRequestTimeout(TBool aValue);
	
	/**
	 * @return true if the peer is a push peer. Push peers are connected
	 * by the engine even if we don't need pieces from them.
	 */
	inline TBool IsPushPeer() const;
	
	/**
	 * Notifies the peer that the local client has downloaded a piece
	 */
	void NotifyThatClientHavePiece(TInt aIndex);
	
	void SetBitFieldLengthL(TInt aLength);
	
	/**
	 * Disconnects the peer if it's connected.
	 */
	void DisconnectL();
	
	/**
	 * Disconnects the peer and marks it deletable.
	 */
	void DeleteL();
	
	/**
	 * Queries the associated connection for the remote address and resets the
	 * stored value (which can be accessed via Address()).
	 *
	 * Used with incoming connections.
	 */
	void ResetAddress();
	
	/**
	 * Sets whether the peer is a push peer or not.
	 */
	inline void SetPushPeer(TBool aIsPushPeer);
	
	/**
	 * Sends a cancel message for the given piece if the peer has an active request for it.
	 */
	void CancelPieceRequestL(CSTPiece* aPiece);
	
	inline void SetReconnectTime(TInt aReconnectTime);
	
	/**
	 * The time when the peer can be reconnected. The value is in seconds and is compared
	 * with the torrent's internal iEllapsedTime member.
	 */
	inline TInt ReconnectTime();
	
	/**
	 * Attaches an incoming connection to the peer
	 */
	void AttachConnectionL(CSTPeerConnection* aConn);
	
	/**
	 * Detaches the active connection
	 */
	void DetachConnection();
	
	inline TBool IsLocal() const;
	
	inline CSTPeerConnection* Connection();
	
	/**
	 * @return the manager class that tracks failed pieces
	 */
	CFailedPieceCollector& FailedPieceCollector() { return *iFailedPieceCollector; }
	
	/**
	 * @return the last time when the peer was connected
	 */
	inline const TTime& LastConnectTime() { return iLastConnectTime; }
	
	/**
	 * Sets the time when the peer was last connected
	 */
	inline void SetLastConnectTime(const TTime& aTime) { 
		iLastConnectTime = aTime; 
		iSuccessfullyConnected = ETrue; 
	}
	
	inline TBool SuccessfullyConnected() { return iSuccessfullyConnected; }
	
private: // constructors

	CSTPeer(const TSockAddr& aAddress, TBool aLocal = EFalse, TBool aPushPeer = EFalse);
	
	void ConstructL(const TDesC8& aPeerId, TInt aBitFieldLength);

	void ConstructL(TInt aBitFieldLength);
	
	void ConstructL(RSocket* aSocket, CSTTorrentManager* aTorrentMgr);

private:

	TSockAddr iAddress;
	
	HBufC8* iPeerId;
	
	CSTPeerConnection* iConnection;
	
	CSTBitField* iBitField;
	
	TInt iErrorCounter;
	
	TBool iDeletable;
	
	TBool iHadRequestTimeout;
	
	/**
	 * Indicates whether the peer is a push peer or not. Push peers are connected
	 * by the engine even if we don't need pieces from them.
	 */
	TBool iPushPeer;
	
	/**
	 * Indicates that a peer is accessible via the local connection
	 */
	TBool iLocal;
	
	/**
	 * The time when the peer can be reconnected. The value is in seconds and is compared
	 * with the torrent's internal iEllapsedTime member.
	 */
	TInt iReconnectTime;
	
	/**
	 * Holds the pieces that were received from this peer and failed the hash check
	 */
	CFailedPieceCollector* iFailedPieceCollector;
	
	/**
	 * The last time when the peer was connected
	 */
	TTime iLastConnectTime;
	
	/**
	 * The peer has been connected at least once sometime
	 */
	TBool iSuccessfullyConnected;
};

// INLINE FUNCTION IMPLEMENTATIONS

inline TSockAddr& CSTPeer::Address() {
	return iAddress;	
}

inline const TDesC8& CSTPeer::PeerId() const {
	if (iPeerId)
		return *iPeerId;
	return KNullDesC8;
}

/*inline CSTBitField* CSTPeer::BitField() {
	return iBitField;
}

inline CSTPiece* CSTPeer::PieceToDownload() {
	if (iConnection)
		return iConnection->PieceToDownload();
	
	return NULL;
}*/

inline const RPointerArray<CSTPieceToDownload>* CSTPeer::PiecesToDownload() const {
	if (iConnection)
		return &(iConnection->PiecesToDownload());
	else
		return NULL;
	
}

inline void CSTPeer::ResetErrorCounter() {
	iErrorCounter = 0;
}

inline TBool CSTPeer::IsDeletable() const {
	return iDeletable;
}

inline TBool CSTPeer::HadRequestTimeout() const {
	return iHadRequestTimeout;
}
	
inline void CSTPeer::SetHadRequestTimeout(TBool aValue) {
	iHadRequestTimeout = aValue;
}

inline const CSTBitField* CSTPeer::BitField() const {
	return iBitField;
}

inline TBool CSTPeer::IsPushPeer() const {
	return iPushPeer;
}

inline void CSTPeer::SetPushPeer(TBool aIsPushPeer) {
	iPushPeer = aIsPushPeer;
}

inline void CSTPeer::SetReconnectTime(TInt aReconnectTime) {
	iReconnectTime = aReconnectTime;
}
	
inline TInt CSTPeer::ReconnectTime() {
	return iReconnectTime;	
}

inline TBool CSTPeer::IsLocal() const {
	return iLocal;
}

inline CSTPeerConnection* CSTPeer::Connection() {
	return iConnection;
}


#endif
