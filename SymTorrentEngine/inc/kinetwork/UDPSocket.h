#ifndef KIUDPSOCKET_H__
#define KIUDPSOCKET_H__

// INCLUDES
#include "SocketBase.h"
#include "UDPSocketReader.h"
#include "UDPSocketWriter.h"

// FORWARD DECLARATIONS
class CKiLogger;

/**
 * CKiUDPSocket
 */
class CKiUDPSocket : public CKiSocketBase
{
public:

	IMPORT_C CKiUDPSocket(CKiLogger* aLog);
	
	IMPORT_C void ConstructL();
	
	/**
	 * Sends a single datagram to a remote host
	 */
	inline void SendDatagramL(TInetAddr& aRemoteAddress, const TDesC8& aPayload);
	
	IMPORT_C ~CKiUDPSocket();
	
	inline void IncIncomingTraffic(TInt aBytes);
	inline void IncOutgoingTraffic(TInt aBytes);
	
	IMPORT_C void LogTrafficStatsL();
	
	/**
	 * Works only if the client once has been started prevously!
	 */
	IMPORT_C void StartReceiving();
	
	IMPORT_C void StopReceiving();
	
protected:

	IMPORT_C void StartReceivingL(TUint aPort);	
	
	IMPORT_C void CloseSocket();
	
	IMPORT_C void OpenSocketL(TInt aNetConnIndex = 0);
	
public:

	virtual void OnReceiveL(const TInetAddr& aRemoteAddress, const TDesC8& aReceivedData) = 0;
	
protected:

	CKiLogger* iLog;

private:
	/**
	 * Active object that receives incoming data
	 */
	CKiUDPSocketReader* iSocketReader;
	
	/**
	 * Active object that sends outgoing data
	 */
	CKiUDPSocketWriter* iSocketWriter;
	
	long unsigned iIncomingTraffic;
	long unsigned iOutgoingTraffic;
};

// INLINE METHOD DEFINITIONS
inline void CKiUDPSocket::SendDatagramL(TInetAddr& aRemoteAddress, const TDesC8& aPayload) {
	iSocketWriter->SendDatagramL(aRemoteAddress, aPayload);
}

inline void CKiUDPSocket::IncIncomingTraffic(TInt aBytes) {
	iIncomingTraffic += aBytes;
}

inline void CKiUDPSocket::IncOutgoingTraffic(TInt aBytes) {
	iOutgoingTraffic += aBytes;
}

#endif // #ifndef KIUDPSOCKET_H__
