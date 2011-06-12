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
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/**
 * ============================================================================
 *  Name     : CNetworkConnection from NetworkConnection.h
 *  Part of  : SymTorrent
 *  Created  : 11.01.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef NETWORKCONNECTION_H__
#define NETWORKCONNECTION_H__

// INCLUDES
#include <Es_sock.h>
#include <in_sock.h>
#include "SocketListenerBase.h"

//FORWARD DECLARATIONS
class CNetworkConnection;
class CNetworkManager;
class MKiAccessPointSupplier;
//class CSocketListenerBase;

// CONSTS
const TInt KBluetoothIapId = -2;

/**
 *  MNetworkConnectionObserver
 */
class MNetworkConnectionObserver
{
public:
	virtual void OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection) = 0;
	
	/**
	 * Called when an active connection is disconnected
	 */
	virtual void OnNetworkConnectionDownL(CNetworkConnection& aConnection) = 0;
};

	
/**
 *  CNetworkConnection
 */
class CNetworkConnection : public CActive
{
public:

	enum TNetworkConnectionState
	{
		ENCUninitialized = 0, // the connection type and IAP id is unset
		ENCInitializing, // asking for access point (this state can be skipped)
		
		ENCStopped,
		ENCStarting,
		ENCStarted	
	};
	
	enum TNetworkConnectionType
	{
		ERConnectionBased,
		EBluetooth
	};
	
	CNetworkConnection(RSocketServ& aSocketServer, CNetworkManager& aNetMgr);
	
	void ConstructL();
	
	~CNetworkConnection();
	
	/**
	 * Will leave if Iap ID is unset.
	 */
	void StartL();
	
	void Close();
	
	
	/**
	 * Sets the connection type and the associated access point id (in case of RConnection-based
	 * connections). The connection must be in unitialized state, otherwise PANIC is raised.
	 */
	IMPORT_C void InitializeL(TNetworkConnectionType aType, TInt aIapId = -1);
	
	IMPORT_C TBool GetIapAndInitializeL(TInt aConnectionId, MKiAccessPointSupplier* aAccessPointSupplier);
	
	/**
	 * Closes the conenctions, removes the access point binding and deletes all socket listeners.
	 */
	IMPORT_C void SetUninitialized();

	inline TBool IsInitialized() const;
	
	inline TInt IapId() const;
	
	inline TNetworkConnectionType Type() const;
	
	inline CNetworkManager& NetMgr();
	
	
	//void CreateSocketListenerL();
	
//	inline CSocketListenerBase* SocketListener(TInt aIndex);
	
//	inline TInt SocketListenerCount() const;
	
	/**
	 * Starts listening for incoming connections onthe given port. 
	 * 
	 * The network connection must already be started (otherwise the function panics)
	 */
	TInt StartListeningL(TUint aPort, MSocketListenerObserver* aObserver);
	
	void StopListening(TUint aPort);
	
	void StopAllListening();
	
	CSocketListenerBase* GetListener(TUint aPort);
	
	const CSocketListenerBase* GetListener(TUint aPort) const;
	
	TBool IsListening(TUint aPort) const;
	
	/**
	 * @return ETrue if there is an active listener
	 */
	TBool IsListening() const;	
	
	inline TNetworkConnectionState State() const;
	
	inline TBool IsStarted() const;
	
	inline RConnection& BaseConnection();
	
	inline void SetNetworkConnectionObserver(MNetworkConnectionObserver* aNetworkConnectionObserver);
	
	inline TBool GotLocalAddress() const;
	
	inline const TSockAddr& LocalAddress();
	
private:

	void ScheduleProgressNotification();
	
	TInt GetLocalIPAddressL(TInetAddr& aAddr);

private:  // from CActive		

	void RunL();

	void DoCancel();						

private:
	
	TNetworkConnectionType 				iType;

	TInt								iIapId;
	
	RSocketServ& 						iSocketServer;
	
	RConnection							iConnection;
	
	TNetworkConnectionState 			iState;
	
	TNifProgressBuf 					iProgress;
	
	MNetworkConnectionObserver* 		iNetworkConnectionObserver;
	
	RPointerArray<CSocketListenerBase> 	iSocketListeners;
	
	RNotifier 							iBluetoothPowerNotifier;
	
	TPckgBuf<TBool>						iBtPowerNotifierResult;
	TPckgBuf<TBool>						iBtPowerNotifierDummy;
	
	TBool iGotLocalAddress;
	
	TSockAddr iLocalAddress;
	
	CNetworkManager& iNetMgr;
};

// INLINE METHOD DEFINITIONS

inline CNetworkConnection::TNetworkConnectionState CNetworkConnection::State() const {
	return iState;		
}

inline TBool CNetworkConnection::IsStarted() const {
	return (iState == ENCStarted);		
}

inline RConnection& CNetworkConnection::BaseConnection() {
	return iConnection;		
}

inline TBool CNetworkConnection::IsInitialized() const {
	return ((iState != ENCUninitialized) && (iState != ENCInitializing));
}

inline TInt CNetworkConnection::IapId() const {
	return iIapId;
}

inline void CNetworkConnection::SetNetworkConnectionObserver(MNetworkConnectionObserver* aNetworkConnectionObserver) {
	iNetworkConnectionObserver = aNetworkConnectionObserver;	
}

inline CNetworkConnection::TNetworkConnectionType CNetworkConnection::Type() const {
	return iType;
}

inline TBool CNetworkConnection::GotLocalAddress() const {
	return iGotLocalAddress;
}

const TSockAddr& CNetworkConnection::LocalAddress() {
	return iLocalAddress;
}

inline CNetworkManager& CNetworkConnection::NetMgr() {
	return iNetMgr;
}

/*inline CSocketListenerBase* CNetworkConnection::SocketListener(TInt aIndex) {
	return iSocketListeners[aIndex];
}

inline TInt CNetworkConnection::SocketListenerCount() const {
	return iSocketListeners.Count();
}*/

#endif

// End of File
