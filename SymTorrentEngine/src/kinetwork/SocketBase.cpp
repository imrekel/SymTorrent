#include "SocketBase.h"
#include "NetworkManager.h"

CKiSocketBase::CKiSocketBase()
{
}

void CKiSocketBase::BaseConstructL()
{	
	RSocket* socket = new (ELeave) RSocket;
	BaseConstructL(socket);
}

void CKiSocketBase::BaseConstructL(RSocket* aSocket)
{
	iNetMgr = NETMGR;
		
	iSocket = aSocket;
}


/*void CKiSocketBase::Detach()
{
	iDetached = ETrue;
}*/


CKiSocketBase::~CKiSocketBase()
{
	iNetMgr->Close(Socket());
	delete iSocket;
}
