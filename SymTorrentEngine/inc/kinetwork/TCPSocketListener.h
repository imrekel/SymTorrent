#ifndef TCPSOCKETLISTENER_H
#define TCPSOCKETLISTENER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "SocketListenerBase.h"

// CLASS DECLARATION

/**
 *  CTCPSocketListener
 * 
 */
class CTCPSocketListener : public CSocketListenerBase
{
public: // Constructors and destructor
	
	~CTCPSocketListener();

	static CTCPSocketListener* NewL(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection);

	static CTCPSocketListener* NewLC(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection);
	
public: // new methods
	
	RConnection& Connection();
	
public: // from CSoscketListenerBase
	
	TInt StartListeningL(TUint aPort);
		
	void StopListeningL();
	
	void RunL();
	
	void DoCancel();

private:

	CTCPSocketListener(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection);

	void ConstructL();
};

#endif // TCPSOCKETLISTENER_H
