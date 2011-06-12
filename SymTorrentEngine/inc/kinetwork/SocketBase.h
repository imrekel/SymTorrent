#ifndef KISOCKETBASE_H_
#define KISOCKETBASE_H_

#include <in_sock.h>
#include "NetworkManager.h"

_LIT(KLocalHost, "127.0.0.1");

/**
 * CKiSocketBase
 *
 * Base class for CKiConnectedSocket and CKiUDPSocket classes.
 */
class CKiSocketBase : public CBase, public MSocketOpenObserver
{
public:

	~CKiSocketBase();

	/**
	 * Detaches (removes ownership) the RSocket handle from the object.
	 */
//	void Detach();

	inline TBool IsDetached() const;

	inline void SocketName(TName& aName);
	
	inline RSocket& Socket() const;
	
	virtual void OnDataSentL(TInt /*aSize*/) {}
	
protected:

	/**
	 * Initializes the instance with an accepted incoming socket.
	 * It takes ownership of the passed RSocket!
	 */
	void BaseConstructL(RSocket* aIncomingConnectionSocket);
	
	void BaseConstructL();

protected:

	IMPORT_C CKiSocketBase();


	IMPORT_C virtual void HandleReadErrorL() = 0;


	IMPORT_C virtual void HandleWriteErrorL() = 0;

	/**
	 * Opens the socket on the given network interface. 
	 * The result is passed to SocketOpenedL (called by the framework).
	 */
	IMPORT_C virtual void OpenSocketL(TInt aNetConnIndex = 0) = 0;

	/**
	 * Closes the socket. Closing the socket MUST be performed with
	 * this method!
	 */
	IMPORT_C virtual void CloseSocket() = 0;
	
protected:

	RSocket*					iSocket; 
	
	CNetworkManager*			iNetMgr;

private: // Member variables

	TBool						iDetached;
	
	friend class CKiConnectedSocketWriter;
	friend class CKiConnectedSocketReader;
	friend class CKiUDPSocketWriter;
	friend class CKiUDPSocketReader;
};

inline RSocket& CKiSocketBase::Socket() const {
	return *iSocket;
}

inline void CKiSocketBase::SocketName(TName& aName) {
	Socket().Name(aName);
}

inline TBool CKiSocketBase::IsDetached() const {
	return iDetached;
}
	
#endif
