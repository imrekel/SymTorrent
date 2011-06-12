#include "SocketListenerBase.h"

CSocketListenerBase::CSocketListenerBase(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection) 
 : 	CActive(EPriorityStandard),
 	iState(ENotListening),
	iSocketServ(aSocketServ),
	iBlankSocket(NULL),
	iNetworkConnection(aNetworkConnection)
{
}

void CSocketListenerBase::SetObserver(MSocketListenerObserver* aObserver)
{
	iObserver = aObserver;
}

void CSocketListenerBase::BaseConstructL()
{
	CActiveScheduler::Add(this); // Add to scheduler
}

CSocketListenerBase::~CSocketListenerBase()
{
	delete iBlankSocket;
}

