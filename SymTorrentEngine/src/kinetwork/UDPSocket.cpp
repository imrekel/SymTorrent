#include "UDPSocket.h"
#include "KiNetworkLog.h"

EXPORT_C CKiUDPSocket::CKiUDPSocket(CKiLogger* aLog)
 : iLog(aLog)
{
	
}

EXPORT_C void CKiUDPSocket::ConstructL()
{
	BaseConstructL();
	
	iSocketReader = new (ELeave) CKiUDPSocketReader(*this, iLog);
	iSocketReader->ConstructL();
	
	iSocketWriter = new (ELeave) CKiUDPSocketWriter(*this, iLog);
	iSocketWriter->ConstructL();
}

EXPORT_C CKiUDPSocket::~CKiUDPSocket()
{
	delete iSocketReader;
	delete iSocketWriter;
}

EXPORT_C void CKiUDPSocket::OpenSocketL(TInt aNetConnIndex)
{
	iNetMgr->OpenUDPSocketL(Socket(), this, aNetConnIndex);
	// little hack
	/*if (((MobileAgent::CMobileAgentDocument*)(CEikonEnv::Static()->EikAppUi()->Document()))->IsIAPSet())
		return iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp, ((MobileAgent::CMobileAgentDocument*)(CEikonEnv::Static()->EikAppUi()->Document()))->CommEngine()->NetworkConnection());
	else	
		return iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp);
		
	return ((MobileAgent::CMobileAgentDocument*)(CEikonEnv::Static()->EikAppUi()->Document()))->CommEngine()->OpenSocket(iSocket);*/
	
//	return NETWORKMGR->OpenSocket(iSocket);
	//return iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp);
}

EXPORT_C void CKiUDPSocket::CloseSocket()
{
	iSocketReader->Cancel();
	iSocketWriter->Cancel();
	iNetMgr->Close(Socket());
}

EXPORT_C void CKiUDPSocket::StartReceivingL(TUint aPort)
{
	iSocketReader->StartL(aPort);
}

EXPORT_C void CKiUDPSocket::StartReceiving()
{
	iSocketReader->Start();
}

EXPORT_C void CKiUDPSocket::StopReceiving()
{
	iSocketReader->Stop();
}


EXPORT_C void CKiUDPSocket::LogTrafficStatsL()
{
#ifdef LOG_TO_FILE
	iLog->WriteL(_L("Incoming traffic: "));
	iLog->WriteL(iIncomingTraffic);
	iLog->WriteLineL(_L(" bytes"));
	
	iLog->WriteL(_L("Outgoing traffic: "));
	iLog->WriteL(iOutgoingTraffic);
	iLog->WriteLineL(_L(" bytes"));
#endif
}
