#ifndef SOCKETLISTENERBASE_H
#define SOCKETLISTENERBASE_H

#include <e32base.h>	
#include <e32std.h>		
#include <Es_sock.h>

class CNetworkConnection;
class CKiLogger;

class MSocketListenerObserver
{
public:
	
	/**
	 * The ownership of the passed socket must be taken! In addition to closing the 
	 * socket, it must also be deleted from the heap!
	 */
	virtual void AcceptSocketL(RSocket* aSocket, TUint aPort, CNetworkConnection& aConnection) = 0;
};

/**
 * CSocketListenerBase
 */
class CSocketListenerBase : public CActive
{
protected:
	
	enum TSocketListenerState
	{
		ENotListening, 
		EStartingListening, 
		EListening
	};
	
public:
	
	void SetObserver(MSocketListenerObserver* aObserver);
	
	~CSocketListenerBase();
	
	inline TSocketListenerState State() const;
	
	inline TBool IsListening() const;
	
	/**
	 * Starts listening.
	 * 
	 * @return KErrNone if the listening has been started successfully
	 */
	virtual TInt StartListeningL(TUint aPort) = 0;
	
	virtual void StopListeningL() = 0;
	
	inline TUint Port() const;

protected:

	CSocketListenerBase(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection);

	void BaseConstructL();

protected:
	
	TSocketListenerState iState; // State of the active object
	
	MSocketListenerObserver* 	iObserver;
	
	RSocketServ&				iSocketServ;
	
	RSocket						iSocketListener;
	
	RSocket* 					iBlankSocket;
	
	TUint						iPort;
	
	CNetworkConnection& 		iNetworkConnection;
};

// INLINE METHOD DEFINITIONS
inline CSocketListenerBase::TSocketListenerState CSocketListenerBase::State() const {
	return iState;
}

inline TBool CSocketListenerBase::IsListening() const {
	return (iState == EListening);
}

inline TUint CSocketListenerBase::Port() const {
	return iPort;
}

#endif // SOCKETLISTENERBASE_H
