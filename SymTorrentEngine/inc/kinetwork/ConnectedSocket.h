#ifndef KITCPSOCKET_H__
#define KITCPSOCKET_H__

// INCLUDES
#include <in_sock.h>
#include "SocketBase.h"
#include "NetworkManager.h"
#include "ConnectedSocketWriter.h"

// FORWARD DECLARATIONS
class CKiConnectedSocketReader;
class CKiConnectedSocketConnector;
class CReadBuffer;

/**
 * CKiConnectedSocket
 * 
 * A connection based socket which can use either TCP or Bluetooth
 */
class CKiConnectedSocket : public CKiSocketBase
{
public:
	/**
	 *
	 * @discussion Tracks the state of this object through the connection process
	 * @value ENotConnected The initial (idle) state
	 * @value EConnecting A connection request is pending with the socket server
	 * @value EConnected TCP connection has been established
	 * @value ELookingUp A DNS lookup request is in progress
	 */
	enum TSocketState 
	{
		ESocketNotConnected,
        ESocketConnecting,
        ESocketConnected,
		ESocketLookingUp
	};
	
	enum TConnectedSocketType
	{
		ETcp,
		EBluetooth
	};
	
public:

	IMPORT_C CKiConnectedSocket(/*TConnectedSocketType aType*/);
	
	/**
	 * Constructs the object from an already connected socket (used for incoming connections)
	 */
	IMPORT_C void ConstructL(RSocket* aSocket);
	
	IMPORT_C void ConstructL();

	IMPORT_C ~CKiConnectedSocket();
	
	/**
	 * Starts connecting to a remote host.
	 */
	IMPORT_C void ConnectL(const TSockAddr& aAddress);
	
	IMPORT_C void StopConnecting();
	
	inline TConnectedSocketType Type() const;
	
public:
	
	IMPORT_C void StartReceiving();
	
	IMPORT_C void StopReceiving();
	
	IMPORT_C void StopSending();
	
	/**
	 * Stops sending, receiving and connecting
	 */
	IMPORT_C void StopAll();

	inline void SendL(const TDesC8& aDes, MKiConnectedSocketWriterObserver* aObserver = NULL);
	
	/**
	 * Puts the the given data to the send buffer. The content of the
	 * send buffer can be sent (flushed) by SendNow()
	 */
	inline void PutToSendBufferL(const TDesC8& aDes, MKiConnectedSocketWriterObserver* aObserver = NULL);
	
	inline void SendNow();
	
	IMPORT_C void SendUrlEncodedL(const TDesC8& aDes);
	
	IMPORT_C void SendUrlEncodedL(const TDesC16& aDes);
	
protected:

	/**
	 * Called by CKiConnectedSocketReader when new data has been read from
	 * the socket. The received data can be read from iRecvBuffer.
	 * 
	 * The read data remains in the buffer until it has been manually
	 * deleted.
	 */
	virtual void OnReceiveL() = 0;	
	
	/**
	 * Called when the socket is connected to the remote host
	 */
	virtual void OnSocketConnectSucceededL() = 0;
	
	/**
	 * Called if the connection to the remote host failed
	 */
	virtual void OnSocketConnectFailedL(TInt aReason) = 0;
	

protected: // from CKiSocketBase

	IMPORT_C virtual void OpenSocketL(TInt aConnIndex);
	
	IMPORT_C virtual void CloseSocket();
	
protected:

	CReadBuffer*					iRecvBuffer;
	
	CKiConnectedSocketReader*       iSocketReader;
	
private:

	CKiConnectedSocketWriter*       iSocketWriter;
	
	CKiConnectedSocketConnector*			iSocketConnector;
	
	TSocketState					iSocketState;
	
	TConnectedSocketType			iType;

	friend class CKiConnectedSocketConnector;
	friend class CKiConnectedSocketReader;	
	friend class CKiConnectedSocketWriter;
};

inline void CKiConnectedSocket::SendL(const TDesC8& aDes, MKiConnectedSocketWriterObserver* aObserver) {
	iSocketWriter->WriteL(aDes, aObserver);
}
inline void CKiConnectedSocket::PutToSendBufferL(const TDesC8& aDes, MKiConnectedSocketWriterObserver* /*aObserver*/) {
	iSocketWriter->WriteWithoutSendingL(aDes);
}
inline void CKiConnectedSocket::SendNow() {
	iSocketWriter->SendNow();
}
// TODO nemkell
inline CKiConnectedSocket::TConnectedSocketType CKiConnectedSocket::Type() const {
	return iType;
}

#endif
