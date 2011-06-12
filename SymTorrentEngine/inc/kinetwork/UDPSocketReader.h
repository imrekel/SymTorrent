#ifndef KIUDPSOCKETREADER_H__
#define KIUDPSOCKETREADER_H__

// INCLUDES
#include <e32base.h>
#include <in_sock.h>

// FORWARD DECLARATIONS
class CKiUDPSocket;
class CKiLogger;

//CONSTS
const TInt KKiMaxDatagramLength = 65527 / 2; //65527; // 16 bit


class CKiUDPSocketReader : public CActive
{
public:

	CKiUDPSocketReader(CKiUDPSocket& aSocket, CKiLogger* aLog);
	
	void ConstructL();
	
	~CKiUDPSocketReader();
	
	/**
	 * Activates the object (starts reading from the socket)
	 */
	void StartL(TUint aPort);
	
	/**
	 * Activates the object and starts receiving from the socket on
	 * the previously set port.
	 */
	void Start();
	
	/**
	 * Stops reading from the socket (cancels the active object)
	 */
	void Stop();
	
protected:

	void IssueRead();

protected: // from CActive

	void RunL();

	void DoCancel();

private:
	
	/**
	 * Address of the remote address that we have received the data from
	 */
	TInetAddr iRemoteAddress;
	
	CKiUDPSocket& iSocket;

	TBuf8<KKiMaxDatagramLength> iBuffer;
	
	CKiLogger* iLog;
	
	TBool iStopped;
};

#endif // #ifndef KIUDPSOCKETREADER_H__
