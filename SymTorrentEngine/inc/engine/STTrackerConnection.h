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

/*
 * ============================================================================
 *  Classes  : CSTTrackerConnection
 *			  
 *  Part of  : SymTorrent
 *  Created  : 14.02.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef SYMTORRENT_STTRACKERCONNECTION_H
#define SYMTORRENT_STTRACKERCONNECTION_H

// INCLUDES
#include <http\mhttpdatasupplier.h>
#include <http\mhttptransactioncallback.h>
#include <http\mhttpauthenticationcallback.h>
#include "NetworkManager.h"
#include "STDefs.h"
#include "STBencode.h"
#include "Timeout.h"
#include "STEnums.h"

// FORWARD DECLARATIONS
class RHTTPSession;
class RHTTPTransaction;
class CSTTorrent;
class CSTTorrentManager;
class CKiLogger;

class MTrackerConnectionObserver
{
public:
	virtual void TrackerConnectionFailedL() = 0;
	virtual void TrackerConnectionSucceededL() = 0;
	virtual void TrackerResponseReceivedL(CSTBencode& aBencodedResponse) = 0;
};

/**
 * CSTTrackerConnection
 */
class CSTTrackerConnection : public CBase, 
							 public MHTTPTransactionCallback, 
							 public MHTTPSessionOpenObserver,
							 public MTimeoutObserver
{
public:
	
	enum TDownloadResult
	{
		EPending = 0,
		EFailed,
		ESucceeded		
	};

	CSTTrackerConnection(CSTTorrent& aTorrent, TTrackerConnectionEvent aEvent = ETrackerEventNotSpecified);

	void ConstructL(const TDesC8& aTrackerAddess);
	
	~CSTTrackerConnection();
	
	void StartTransactionL();			

	void Cancel();
	
	inline TBool IsRunning() const;
	
	inline TDownloadResult Result() const;
	
	inline TTrackerConnectionEvent Event() const;
	
	/**
	 * Sets the connection's observer
	 */
	void SetObserver(MTrackerConnectionObserver* aObserver) { iObserver = aObserver; }
					
private:

	void SetHeaderL(RHTTPHeaders aHeaders, TInt aHdrField, 
					const TDesC8& aHdrValue);
					
	void OnTimerL();
	
	void SetFailed();
	
	/**
	 * Creates the trackers uri for the http transaction.
	 */
	void CreateUriL();
	
	CSTPreferences* Preferences();
	
	CSTTorrentManager* TorrentMgr();
	
private:  // from MTimeoutObserver
	
	TBool HandleTimeIsUp(CTimeout* aTimeout, TInt aStatus);

private: // from MHTTPSessionEventCallback

	void MHFRunL(RHTTPTransaction aTransaction, const THTTPEvent& aEvent);
	
	TInt MHFRunError(	TInt aError, 
						RHTTPTransaction aTransaction, 
						const THTTPEvent& aEvent);

private:
					
	void HTTPSessionOpenedL(TBool aResult, RHTTPSession& aHTTPSession);

private:

	CSTTorrent& 			iTorrent;	
		
	RHTTPSession			iSession;
	
	RHTTPTransaction		iTransaction;
	
	TBool					iRunning;	// ETrue, if HTTP transaction is running
	
	/**
	 * Tracker address (htt://hostname/path)
	 */
	HBufC8*					iAddress;
	/**
	 * Full URL with announce parameters
	 */
	HBufC8*					iUri;
	
	HBufC8*					iReceiveBuffer;
	
	CNetworkManager* 		iNetMgr;
	
	TTrackerConnectionEvent	iEvent;
	
	CKiLogger*				iLog;
	
	MTrackerConnectionObserver* iObserver;
	
	CTimeout* iConnectionTimeout;
	

	
	friend class CSTTorrent; // for calling OnTimerL()	
};	


inline TBool CSTTrackerConnection::IsRunning() const { 
	return iRunning; 
};

inline TTrackerConnectionEvent CSTTrackerConnection::Event() const {
	return iEvent;
}

#endif
