/*
 ============================================================================
 Name		: TCPSocketListener.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CTCPSocketListener implementation
 ============================================================================
 */

#include "TCPSocketListener.h"
#include "NetworkConnection.h"
#include "KiNetwork.pan"
#include "NetworkManager.h"
#include "KiNetworkLog.h"
#include <in_sock.h>

#define iLog iNetworkConnection.NetMgr().LogL()

CTCPSocketListener::CTCPSocketListener(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection)
 : CSocketListenerBase(aSocketServ, aNetworkConnection)
{
	// No implementation required
}

CTCPSocketListener::~CTCPSocketListener()
{
	StopListeningL();
}

CTCPSocketListener* CTCPSocketListener::NewLC(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection)
{
	CTCPSocketListener* self = new (ELeave) CTCPSocketListener(aSocketServ, aNetworkConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CTCPSocketListener* CTCPSocketListener::NewL(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection)
{
	CTCPSocketListener* self = CTCPSocketListener::NewLC(aSocketServ, aNetworkConnection);
	CleanupStack::Pop(); // self;
	return self;
}

void CTCPSocketListener::ConstructL()
{
	BaseConstructL();
}

TInt CTCPSocketListener::StartListeningL(TUint aPort)
{
	HLWRITELN(iLog, _L("[CTCPSocketListener] StartListeningL begin"));
	
	if (iState != ENotListening)
		User::Panic(KLitKiNetworkPanic, KPanSocketListenerIsAlreadyListening);
	
	iPort = aPort;
	
	User::LeaveIfError(iSocketListener.Open(iSocketServ, KAfInet, KSockStream, 
		KProtocolInetTcp, iNetworkConnection.BaseConnection()));
		
	TInetAddr addr;
	addr.SetPort(iPort);
	
	HLWRITELN(iLog, _L("Bind start"));
	TInt err = iSocketListener.Bind(addr);				
	if (err != KErrNone)
		return err;
	HLWRITELN(iLog, _L("Bind end"));
	
	HLWRITELN(iLog, _L("Listen start"));
	err = iSocketListener.Listen(5);
	if (err != KErrNone)
		return err;
	HLWRITELN(iLog, _L("Listen end"));
	
	delete iBlankSocket;
	iBlankSocket = NULL;
	
	iBlankSocket = new RSocket();
	iBlankSocket->Open(iSocketServ);
	iSocketListener.Accept(*iBlankSocket, iStatus); 
	iState = EListening;
	SetActive();
	
	HLWRITELN(iLog, _L("[CTCPSocketListener] StartListeningL end"));
	
	return KErrNone;
}
	
void CTCPSocketListener::StopListeningL()
{
	Cancel();
	iSocketListener.Close();
	delete iBlankSocket;
	iBlankSocket = NULL;

	iState = ENotListening;
}

void CTCPSocketListener::RunL() 
{
	if (iState == EListening)
	{
		if (iStatus.Int() == KErrNone)
		{
			if (iObserver)
				iObserver->AcceptSocketL(iBlankSocket, iPort, iNetworkConnection);
			else
				delete iBlankSocket;

			iBlankSocket = NULL;
			iBlankSocket = new RSocket();
			iBlankSocket->Open(iSocketServ);
			iSocketListener.Accept(*iBlankSocket, iStatus); 
			SetActive();
		}
		else
			StopListeningL();
	}
}
	
void CTCPSocketListener::DoCancel()
{
	iSocketListener.CancelAll();
}	

RConnection& CTCPSocketListener::Connection() 
{
	return iNetworkConnection.BaseConnection();
}
