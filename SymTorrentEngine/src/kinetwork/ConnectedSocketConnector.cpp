#include "ConnectedSocketConnector.h"
#include "ConnectedSocket.h"

CKiConnectedSocketConnector::CKiConnectedSocketConnector(CKiConnectedSocket& aSocket)
 : CActive(EPriorityStandard), iSocket(aSocket)
{	
}

void CKiConnectedSocketConnector::ConstructL()
{
	CActiveScheduler::Add(this);
}

CKiConnectedSocketConnector::~CKiConnectedSocketConnector()
{
	Cancel();
}

void CKiConnectedSocketConnector::ConnectL(const TSockAddr& aAddress)
{
	if (!IsActive())
	{
		iAddress = aAddress;
	
		iSocket.Socket().Connect(iAddress, iStatus);
		SetActive();
	}
	else
	{
		iSocket.OnSocketConnectFailedL(KErrGeneral);
	}

}

void CKiConnectedSocketConnector::RunL()
{
	if (iStatus == KErrNone) 	// Connected successfully
		iSocket.OnSocketConnectSucceededL();
	else 	// Connection failed
		iSocket.OnSocketConnectFailedL(iStatus.Int());
}

void CKiConnectedSocketConnector::DoCancel()
{
	iSocket.Socket().CancelConnect();
}
