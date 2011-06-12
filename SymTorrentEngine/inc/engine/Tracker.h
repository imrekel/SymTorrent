/*
 ============================================================================
 Name		: Tracker.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CTracker declaration
 ============================================================================
 */

#ifndef TRACKER_H
#define TRACKER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "Timeout.h"
#include "STEnums.h"
#include "STTrackerConnection.h"
#include "NetworkManager.h"

// CLASS DECLARATION
class CSTTorrent;
class CTrackerManager;
class CTracker;

class MTrackerObserver
{
public:
	virtual void TrackerAnnounceFinishedL(CTracker& aTracker) = 0;
};

/**
 *  CTracker
 * 
 */
class CTracker: public CBase, 
				public MTimeoutObserver,
				public MTrackerConnectionObserver,
				public MKiNetworkManagerObserver
{
public:
	// Constructors and destructor
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CTracker(CTrackerManager& aTrackerManager);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& aAddress);

	/**
	 * Destructor.
	 */
	~CTracker();
	
public:
	
	/**
	 * Connects to the tracker and start timeout afterwards
	 */
	void AnnounceL(TTrackerConnectionEvent aEvent = ETrackerEventNotSpecified);
	
	/**
	 * Disconnects if there is an ongoing connection and stops reconnect timeout
	 */
	void Close();
	
	/**
	 * @return currently connecting or waiting until the next connection retry
	 */
	TBool IsAnnouncing() { return iIsAnnouncing; }
	
	/**
	 * @return The tracker has been successfully contacted at least once
	 */
	TBool IsAnnounced() { return iIsAnnounced; }
	
	/**
	 * Sets the tracker's observer.
	 */
	void SetObserver(MTrackerObserver* aObserver) { iObserver = aObserver; }
	
	/**
	 * @return the tracker's address
	 */
	const TDesC8& Address() const { return *iAddress; }
	
private: // from MTrackerConnectionObserver
	
	void TrackerConnectionFailedL();
	void TrackerConnectionSucceededL();
	void TrackerResponseReceivedL(CSTBencode& aBencodedResponse);
	
private: // from MTimeoutObserver
	
	TBool HandleTimeIsUp(CTimeout* aTimeout, TInt aStatus);
	
private: // from MKiNetworkManagerObserver
	
	void NetworkManagerEventReceivedL(TKiNetworkManagerEvent aEvent);
	
	void OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection, TInt aConnectionIndex);

	void OnNetworkConnectionDownL(CNetworkConnection& aConnection, TInt aConnectionIndex);
	
private:

	CTrackerManager& iTrackerManager;			// The tracker manager for this tracker
	CSTTrackerConnection* iConnection;			// HTTP request to the tracker
	TInt iConnectionFailureCount;				// Number of times connecting to the tracker has been failed
	CTimeout* iConnectTimeout;					// Issues connecting to the tracker
	TInt iTrackerInterval;
	HBufC8* iAddress;							// Tracker address (URL)
	TTrackerConnectionEvent iCurrentEvent;		// The event that is being sent to the tracker
	TBool iIsAnnouncing;						// Currently connecting or waiting until the next connection retry
	TBool iIsAnnounced;							// The tracker has been successfully contacted at least once
	MTrackerObserver* iObserver;
};

#endif // TRACKER_H
