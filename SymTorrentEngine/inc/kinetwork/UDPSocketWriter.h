#ifndef KIUDPSOCKETWRITER_H__
#define KIUDPSOCKETWRITER_H__

// INCLUDES
#include <e32base.h>
#include <in_sock.h>

// FORWARD DECLARATIONS
class CKiUDPSocket;
class CWriteBuffer;
class CKiLogger;

/**
 * CKiDatagram
 *
 * A single datagram that will be sent to a remote address.
 * Stores a network address and the payload to be sent.
 */
class CKiDatagram
{
public:

	static CKiDatagram* NewLC(TInetAddr& aAddress, const TDesC8& aPayLoad);
	static CKiDatagram* NewL(TInetAddr& aAddress, const TDesC8& aPayLoad);	
	
	~CKiDatagram() { delete iPayLoad; }
	
	inline TInetAddr& Address();
	
	inline const TDesC8& PayLoad() const;
	
protected:

	CKiDatagram(TInetAddr& aAddress)
	 : iAddress(aAddress)
	{}
	 
	void ConstructL(const TDesC8& aPayLoad);
		
private:

	HBufC8* iPayLoad;
	
	TInetAddr iAddress;
};

/**
 * CKiUDPSocketWriter
 */
class CKiUDPSocketWriter : public CActive
{
public:

	CKiUDPSocketWriter(CKiUDPSocket& aSocket, CKiLogger* aLog);
	
	void ConstructL();
	
	~CKiUDPSocketWriter();
	
	/**
	 * Sends a single datagram to a remote address
	 */
	IMPORT_C void SendDatagramL(TInetAddr& aRemoteAddress, const TDesC8& aDatagram);
	
protected:

	void IssueWrite();

protected: // from CActive

	void RunL();

	void DoCancel();

private:
	
	RPointerArray<CKiDatagram> iDatagrams;
	
	CKiUDPSocket& iSocket;
	
	CKiLogger* iLog;
};

// INLINE METHOD DEFINITIONS
inline TInetAddr& CKiDatagram::Address() {
	return iAddress;
}
	
inline const TDesC8& CKiDatagram::PayLoad() const {
	return *iPayLoad;
}

#endif // #ifndef KIUDPSOCKETWRITER_H__
