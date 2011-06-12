#ifndef BLUETOOTHSOCKETLISTENER_H
#define BLUETOOTHSOCKETLISTENER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <bt_sock.h>
#include <btsdp.h>

#include "SocketListenerBase.h"

// CLASS DECLARATION

/**
 *  CBluetoothSocketListener
 * 
 */
class CBluetoothSocketListener  : public CSocketListenerBase
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CBluetoothSocketListener();

	/**
	 * Two-phased constructor.
	 */
	static CBluetoothSocketListener* NewL(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection);

	/**
	 * Two-phased constructor.
	 */
	static CBluetoothSocketListener* NewLC(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection);
	
private: // from CSoscketListenerBase
	
	TInt StartListeningL(TUint aPort = 0);
		
	void StopListeningL();
	
	void RunL();
	
	void DoCancel();
	
private: // New methods
	
	/**
	 * Sets the Bluetooth security settings for the given port
	 */
	void SetSecurityL(TBTSockAddr& aAddr, TInt aPort);
	
	void StartServiceAdvertiserL(TInt aChannel);
	
	void StopServiceAdvertiserL();
	
	void UpdateServiceAvailabilityL(TBool aAvailable);

private:
	
	CBluetoothSocketListener(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection);

	void ConstructL();
	
private:
	
	// service discovery protocol session
	RSdp iSdp;
	// service discovery database (sdp)
	RSdpDatabase iSdpDB;
	// service record
    TSdpServRecordHandle iServiceRecord;
	// service record state
	TInt iRecordState;
};

#endif // BLUETOOTHSOCKETLISTENER_H
