//#include "STTorrentManager.h"
#include "NetworkManager.h"
//#include "STPreferences.h"
//#include "STTBaseData.h"
#include "SocketListenerBase.h"
#include "TCPSocketListener.h"
#include "BluetoothManager.h"
#include <CommDbConnPref.h>
#include <HttpStringConstants.h>
#include <bt_sock.h>
#include <http\RHTTPSession.h>
#include <http\RHTTPHeaders.h>
#include <eikenv.h>
#include <in_sock.h>
#include "KiNetworkLog.h"
#include "MutablePointerArrayIterator.h"

CNetworkManager::CNetworkManager(/*CSTTorrentManager* aTorrentMgr*/)
 : CCoeStatic(KUidNetworkManagerSingleton)//, iLastProxyConnectionAttemptResult(EProxyConnectionPending)//,
   //iTorrentMgr(aTorrentMgr)
{
}

void CNetworkManager::ConstructL()
{
	#ifdef LOG_TO_FILE
		CKiLogManager::InitializeL();
	#endif
		
	User::LeaveIfError(iSocketServ.Connect(255));
	
	// primary connection (long range), index = 0
	CreateNetworkConnectionL(CNetworkConnection::ERConnectionBased);

	//iProxyParameters = CSTTBaseData::NewL();
	//iProxyParameters->SetConnectionPort(prefs->ProxyConnectionPort());
	//iProxyParameters->SetServicePort(prefs->ProxyServicePort());
	//iProxyParameters->SetProxyAddress(prefs->ProxyHostName());
	
	//TBuf<20> postfix = _L("_SymTorrent");
	//iProxyConnector = 
		//CSTTConnector::NewL(&iSocketServ, this, iProxyParameters, postfix, NULL, this);
	
	//iListenerParameters = CSTTBaseData::NewL();
	//iListenerParameters->SetConnectionPort(prefs->IncomingPort());
			
	//iSocketListener =
		//CSTTConnector::NewL(&iSocketServ, this, iListenerParameters, postfix);
		
//	SetNetworkConnectionId(prefs->AccessPointId());
	
//	prefs->AddPreferencesObserverL(this);
}

CKiLogger* CNetworkManager::LogL()
{
	#ifdef LOG_TO_FILE
	if (!iLog)
	{
		iLog = CKiLogManager::Instance()->GetLoggerL(TUid::Uid(KINETWORK_UID));
	}
	#endif
	
	return iLog;
}

EXPORT_C TInt CNetworkManager::CreateNetworkConnectionL(CNetworkConnection::TNetworkConnectionType aType, 
														TInt32 aIapId)
{
	CNetworkConnection* networkConnection = new (ELeave) CNetworkConnection(iSocketServ, *this);
	CleanupStack::PushL(networkConnection);
	networkConnection->ConstructL();
	
	if ((aType == CNetworkConnection::EBluetooth) || (aIapId != -1))
		networkConnection->InitializeL(aType, aIapId);
	
	iNetworkConnections.AppendL(networkConnection);
	CleanupStack::Pop();		
	
	networkConnection->SetNetworkConnectionObserver(this);
	
	return iNetworkConnections.Count() - 1;
}

EXPORT_C void CNetworkManager::Initialize(	/*TInt aProxyConnectionPort, 
						 					TInt aProxyServicePort,
						 					const TDesC& aProxyHostName,
						 					TInt aIncomingPort,*/
						 					/*TUint32 aDefaultAccesPointId,*/
						 					CKiLogger* aLog)
{
/*	iProxyParameters->SetConnectionPort(aProxyConnectionPort);
	iProxyParameters->SetServicePort(aProxyServicePort);
	iProxyParameters->SetProxyAddress(aProxyHostName);
	iListenerParameters->SetConnectionPort(aIncomingPort);*/		
	
	if (aLog)
		SetLog(aLog);
}


EXPORT_C CNetworkManager* CNetworkManager::InstanceL()
{
	CNetworkManager* instance = static_cast<CNetworkManager*> 
		( CCoeEnv::Static( KUidNetworkManagerSingleton ) );
	
	if (instance == 0)
	{
		instance = new (ELeave) CNetworkManager;

		CleanupStack::PushL( instance ); 
		instance->ConstructL(); 
		CleanupStack::Pop(); 
	}

	return instance;
}
 

CNetworkManager::~CNetworkManager()
{	
//	delete iProxyConnector;
//	delete iProxyParameters;
//
//	delete iSocketListener;
//	delete iListenerParameters;
			
	iSockets.ResetAndDestroy();
	iHostResolvers.ResetAndDestroy();
	iHTTPSessions.ResetAndDestroy();
	iListeners.ResetAndDestroy();
	
	//iProxyConnectionObservers.Reset();	
	
	//iNetworkConnectionObservers.Reset();
	
	iSocketAccepters.Reset();
	
	iNetworkConnections.ResetAndDestroy();
	
	iSocketServ.Close();
	
	iObservers.Reset();
	
	delete iBtManager;
	
	#ifdef LOG_TO_FILE
		CKiLogManager::Free();
	#endif
}


/*EXPORT_C void CNetworkManager::ConnectProxyL()
{
	if (ProxyConnectorState() == EConOffline)
	{
		iLastProxyConnectionAttemptResult = EProxyConnectionPending;
		
		#if defined(__WINS__) && !defined(EKA2)
			iProxyConnector->StartL(EConProxy);
			iTorrentMgr->NotifyEngineStateObserverL();
		#else
			if (IsNetworkConnectionStarted())
			{
				iProxyConnector->StartL(EConProxy, &(iNetworkConnection->Connection()));
				ReportEventL(EKiEventProxyConnected);
			//	iTorrentMgr->NotifyEngineStateObserverL();				
			}				
			else
			{
				iConnectingProxy = ETrue;
				StartNetworkConnectionL();
			}
		#endif				
	}	
}


EXPORT_C void CNetworkManager::DisconnectProxy()
{
	iProxyConnector->Stop();
	ReportEventL(EKiEventProxyDisconnected);
	//iTorrentMgr->NotifyEngineStateObserverL();
}*/


EXPORT_C void CNetworkManager::StartListeningL(TInt aConnectionIndex, TUint aPort)
{
	LWRITE(LogL(), _L("[Network] Listen issued for connection "));
	LWRITELN(LogL(), aConnectionIndex);
	
	CNetworkConnection& conn = *iNetworkConnections[aConnectionIndex];
	
	if (conn.IsStarted())
	{		
		if (conn.StartListeningL(aPort, this) != KErrNone)
		{
			LWRITELN(iLog, _L("[Network] Listening failed"));
		}					
		else
			LWRITELN(iLog, _L("[Network] Listening"));
		
		// TODO Report error?
		
		ReportEventL(EKiEventListeningStarted);
	}				
	else
	{
		TSocketListenerWithPort* listener = new (ELeave) TSocketListenerWithPort;
		listener->iPort = aPort;
		listener->iConnection = &conn;
		
		iListeners.AppendL(listener);

		StartNetworkConnectionL(aConnectionIndex);
	}
}


EXPORT_C void CNetworkManager::StopListeningL(TInt aConnectionIndex, TUint aPort)
{
	CNetworkConnection& conn = *iNetworkConnections[aConnectionIndex];

	if (conn.IsListening(aPort))
	{
		LWRITELN(LogL(), _L("[NetworkManager] Listening stopped"));
		
		conn.StopListening(aPort);
		
		ReportEventL(EKiEventListeningStopped);
	}
}


EXPORT_C void CNetworkManager::StartNetworkConnectionL(TInt aIndex)
{
	LWRITELN(LogL(), _L("[Network] Starting network connection"));

	CNetworkConnection& conn = *iNetworkConnections[aIndex];
	
	if ((conn.State() == CNetworkConnection::ENCStopped) || 
		(conn.State() == CNetworkConnection::ENCUninitialized))
	{		
		TBool initialized = ETrue;
		
		if (!conn.IsInitialized() && (conn.State() != CNetworkConnection::ENCInitializing))
		{
			LWRITELN(LogL(), _L("[Network] Asking for access point"));
			//initialized = GetIapIdL(conn);
			
			initialized = conn.GetIapAndInitializeL(aIndex, iAccessPointSupplier);
			
			if (!initialized)
			{
				LWRITELN(LogL(), _L("[Network] Access point selection dialog canceled"));
				OnNetworkConnectionStartedL(EFalse, conn);
				return;
			}				
		}
		else
			LWRITELN(LogL(), _L("[Network] Access point is already set"));
			
		LWRITE(LogL(), _L("[Network] Access point id: "));
		LWRITELN(LogL(), conn.IapId());
		
		if (initialized)
		{				
			LWRITE(LogL(), _L("[Network] Starting network connection: "));
			LWRITELN(LogL(), conn.IapId());
			conn.StartL();
		}
	}
}


EXPORT_C void CNetworkManager::CloseNetworkConnection(TInt aIndex)
{
	CNetworkConnection& conn = *iNetworkConnections[aIndex];
	
	if (conn.State() != CNetworkConnection::ENCStopped)
	{
		LWRITELN(LogL(), _L("[Network] Closing network connection"));
		conn.Close();
	}	
}


EXPORT_C void CNetworkManager::OpenHostResolverL(RHostResolver& aResolver, MHostResolverOpenObserver* aObserver, TInt aConnIndex)
{
	#if !defined(__WINS__) || defined(EKA2)
	{	
		CNetworkConnection& conn = *iNetworkConnections[aConnIndex];
		
		if (!conn.IsStarted())
		{
			THostResolverWithObserver* resolver = new (ELeave) THostResolverWithObserver;
			resolver->iHostResolver = &aResolver;
			resolver->iOpenObserver = aObserver;
			resolver->iConnection = &conn;
			
			iHostResolvers.AppendL(resolver);
			StartNetworkConnectionL(aConnIndex);			
		}
		else
		{
			if (aResolver.Open(iSocketServ, KAfInet, 
				KProtocolInetTcp, conn.BaseConnection()) == KErrNone)
				aObserver->HostResolverOpenedL(ETrue, aResolver);
			else
				aObserver->HostResolverOpenedL(EFalse, aResolver);
		}
		
		return;
	}
	#else
	{	
		if (aResolver.Open(iSocketServ, KAfInet, KProtocolInetTcp) == KErrNone)
			aObserver->HostResolverOpenedL(ETrue, aResolver);
		else
			aObserver->HostResolverOpenedL(EFalse, aResolver);
	}
	#endif
}


EXPORT_C void CNetworkManager::OpenConnectedSocketL(RSocket& aSocket, MSocketOpenObserver* aObserver, TInt aConnIndex)
{
	OpenSocketL(aSocket, EKiSocketConnected, aObserver, aConnIndex);
}


EXPORT_C void CNetworkManager::OpenUDPSocketL(RSocket& aSocket, MSocketOpenObserver* aObserver, TInt aConnIndex)
{
	OpenSocketL(aSocket, EKiSocketUDP, aObserver, aConnIndex);
}


EXPORT_C void CNetworkManager::OpenSocketL(RSocket& aSocket, TKiSocketType aSocketType, MSocketOpenObserver* aObserver, TInt aConnIndex)
{
	#if !defined(__WINS__) || defined(EKA2)
	{
		CNetworkConnection& conn = *iNetworkConnections[aConnIndex];		
		if (!conn.IsStarted())
		{
			TSocketWithObserver* socket = new (ELeave) TSocketWithObserver;
			socket->iSocket = &aSocket;
			socket->iOpenObserver = aObserver;
			socket->iSocketType = aSocketType;
			socket->iConnection = &conn;
			
			iSockets.AppendL(socket);
			StartNetworkConnectionL(aConnIndex);			
		}
		else
		{
			DoOpenSocketL(aSocket, aSocketType, conn, aObserver);
		}
		
		return;
	}
	#else
	{
		TInt result = KErrNone;
		
		if (aSocketType == EKiSocketTCP)
			result = aSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp);
		else
			result = aSocket.Open(iSocketServ, KAfInet, KSockDatagram, KProtocolInetUdp);
			
		if (result == KErrNone)
			aObserver->SocketOpenedL(ETrue, aSocket);
		else
			aObserver->SocketOpenedL(EFalse, aSocket);
	}
	#endif
}

void CNetworkManager::DoOpenSocketL(RSocket& aSocket, 
									TKiSocketType aSocketType,
									CNetworkConnection& aConnection, 
									MSocketOpenObserver* aObserver)
{
	TInt result = KErrNone;
	
	if (aSocketType == EKiSocketConnected)
	{
		if (aConnection.Type() == CNetworkConnection::ERConnectionBased) // TCP
		{
			result = aSocket.Open(iSocketServ, KAfInet, KSockStream, 
				KProtocolInetTcp, aConnection.BaseConnection());
		}
		else // Bluetooth
		{				
		//	TProtocolDesc pdesc;
		//	_LIT(KL2Cap, "BTLinkManager");
		//	User::LeaveIfError(iSocketServ.FindProtocol(KL2Cap(), pdesc));
			
			result = aSocket.Open(iSocketServ, KBTAddrFamily, KSockStream, KRFCOMM);
		}		
	}
	else // UDP
		result = aSocket.Open(iSocketServ, KAfInet, KSockDatagram, 
			KProtocolInetUdp, aConnection.BaseConnection());
		
	if (result == KErrNone)
		aObserver->SocketOpenedL(ETrue, aSocket);
	else
		aObserver->SocketOpenedL(EFalse, aSocket);
}


EXPORT_C void CNetworkManager::OpenHTTPSessionL(RHTTPSession& aSession, MHTTPSessionOpenObserver* aObserver, TInt aConnIndex)
{
	LWRITELN(LogL(), _L("[NetworkManager] Opening HTTP session"));
	#if !defined(__WINS__) || defined(EKA2)
	{
		CNetworkConnection& conn = *iNetworkConnections[aConnIndex];
		if (!conn.IsStarted())
		{
			THTTPSessionWithObserver* session = new (ELeave) THTTPSessionWithObserver;
			session->iSession = &aSession;
			session->iOpenObserver = aObserver;
			session->iConnection = &conn;
			
			iHTTPSessions.AppendL(session);
			LWRITELN(LogL(), _L("HTTP session registered"));
			StartNetworkConnectionL(aConnIndex);			
		}
		else
		{
			aSession.OpenL();
			SetHTTPSessionInfoL(aSession, conn);
			aObserver->HTTPSessionOpenedL(ETrue, aSession);
		}
			
	}
	#else
		aSession.OpenL();
		aObserver->HTTPSessionOpenedL(ETrue, aSession);
	#endif
}

void CNetworkManager::SetHTTPSessionInfoL(RHTTPSession& aSession, CNetworkConnection& aNetworkConnection)
{
	LWRITELN(LogL(), _L("[NetworkManager] Setting HTTP connection info"));
	
	if (aNetworkConnection.Type() != CNetworkConnection::EBluetooth)
	{
		RStringPool strP = aSession.StringPool();
		RHTTPConnectionInfo connInfo = aSession.ConnectionInfo();

		connInfo.SetPropertyL(strP.StringF(HTTP::EHttpSocketServ, RHTTPSession::GetTable()),
			THTTPHdrVal(iSocketServ.Handle()) );

		TInt connPtr = REINTERPRET_CAST(TInt, &(aNetworkConnection.BaseConnection()));

		connInfo.SetPropertyL ( strP.StringF(HTTP::EHttpSocketConnection, RHTTPSession::GetTable()),
			THTTPHdrVal(connPtr) );
	}
}


//TBool CNetworkManager::GetIapIdL(CNetworkConnection& aConnection)
//{
//	LWRITELN(LogL(), _L("[NetworkManager] Getting Iap ID"));
//					
//	if (!aConnection.IsInitialized())
//	{
//		TInt connIndex = 0;
//		for (TInt i=0; iNetworkConnections.Count(); i++)
//		{
//			if (iNetworkConnections[i] == &aConnection)
//			{
//				connIndex = i;
//				break;
//			}
//		}				
//			
//		TInt32 iap;
//		TBuf<150> iapName;
//		if (iAccessPointSupplier->GetIapIdL(iap, iapName, connIndex))
//		{
//			if (iap == KBluetoothIapId)
//			{
//				aConnection.InitializeL(CNetworkConnection::EBluetooth, iap);
//			}
//			else
//				aConnection.InitializeL(CNetworkConnection::ERConnectionBased, iap);
//			
//			return ETrue;
//		}
//						
//		return EFalse;		
//	}
//	
//	return ETrue;	
//}


void CNetworkManager::OnNetworkConnectionDownL(CNetworkConnection& aConnection)
{
	HLWRITELN(LOG, _L("[CNetworkManager] OnNetworkConnectionDownL begin"));
	
	#ifdef LOG_TO_FILE
		TBuf<256> logBuf;
		logBuf.Format(_L("[NetworkManager] RConnection (IAP: %d) DOWN"), aConnection.IapId());
		
		LWRITELN(LogL(), logBuf);
	#endif
		
	// notifying the observers
	TInt connIndex = iNetworkConnections.Find(&aConnection);
	for (TInt i=0; i<iObservers.Count(); i++)
		iObservers[i]->OnNetworkConnectionDownL(aConnection, connIndex);
	
	HLWRITELN(LOG, _L("[CNetworkManager] OnNetworkConnectionDownL end"));
}


void CNetworkManager::OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection)
{
	HLWRITELN(LOG, _L("[CNetworkManager] OnNetworkConnectionStartedL begin"));
	
	#ifdef LOG_TO_FILE
		TBuf<256> logBuf;
		if (aConnection.Type() == CNetworkConnection::EBluetooth)
			logBuf.Format(_L("[NetworkManager] Bluetooth (IAP: %d) "), aConnection.IapId());
		else
			logBuf.Format(_L("[NetworkManager] RConnection (IAP: %d) "), aConnection.IapId());
		
		if (aResult)
			logBuf.Append(_L("STARTED"));
		else
			logBuf.Append(_L("FAILED TO START"));
		
		LWRITELN(LogL(), logBuf);
	#endif
	
	TInt connIndex = iNetworkConnections.Find(&aConnection);
	for (TInt i=0; i<iObservers.Count(); i++)
		iObservers[i]->OnNetworkConnectionStartedL(aResult, aConnection, connIndex);

	RMutablePointerArrayIterator<TSocketWithObserver> sockets;
	sockets.OpenL(iSockets);
	CleanupClosePushL(sockets);
	while (sockets.HasNext())
	{
		TSocketWithObserver* socket = sockets.NextL();
		if (socket->iConnection == &aConnection)
		{
			TBool result = aResult;			
			if (result)
			{
				DoOpenSocketL((*socket->iSocket), socket->iSocketType, 
					aConnection, socket->iOpenObserver);				
			}
				
			socket->iOpenObserver->SocketOpenedL(result, *socket->iSocket);
			if (sockets.Remove(socket) == KErrNone)
				delete socket;
		}	
	}
	CleanupStack::PopAndDestroy(&sockets);

	
	RMutablePointerArrayIterator<THostResolverWithObserver> hostResolvers;
	hostResolvers.OpenL(iHostResolvers);
	CleanupClosePushL(hostResolvers);
	while (hostResolvers.HasNext())
	{
		THostResolverWithObserver* resolver = hostResolvers.NextL();
		if (resolver->iConnection == &aConnection)
		{
			TBool result = aResult;
			if (result)
				result = ((*resolver->iHostResolver).Open(iSocketServ, KAfInet, KProtocolInetTcp, aConnection.BaseConnection()) == KErrNone);
		
			resolver->iOpenObserver->HostResolverOpenedL(result, *resolver->iHostResolver);
			
			if (hostResolvers.Remove(resolver) == KErrNone)
				delete resolver;
		}
	}	
	CleanupStack::PopAndDestroy(&hostResolvers);
	
	
	RMutablePointerArrayIterator<THTTPSessionWithObserver> httpSessions;
	httpSessions.OpenL(iHTTPSessions);
	CleanupClosePushL(httpSessions);
	while (hostResolvers.HasNext())
	{
		THTTPSessionWithObserver* session = httpSessions.NextL();
		if (session->iConnection == &aConnection)
		{
			TBool result = aResult;
			if (result)
			{
				(*session->iSession).OpenL();
				SetHTTPSessionInfoL(*session->iSession, aConnection);
			}
			
			session->iOpenObserver->HTTPSessionOpenedL(result, *session->iSession);
			
			if (httpSessions.Remove(session) == KErrNone)
				delete session;
		}
	}
	CleanupStack::PopAndDestroy(&httpSessions);
		
	
	RMutablePointerArrayIterator<TSocketListenerWithPort> listeners;
	listeners.OpenL(iListeners);
	CleanupClosePushL(listeners);
	while (listeners.HasNext())
	{
		TSocketListenerWithPort* listener = listeners.NextL();
		if (listener->iConnection == &aConnection)
		{
			TBool result = aResult;
			if (result)
				aConnection.StartListeningL(listener->iPort, this);						
			
			if (listeners.Remove(listener) == KErrNone)
				delete listener;
		}
	}
	CleanupStack::PopAndDestroy(&listeners);
	
	//iSockets.Reset();
	//iHostResolvers.Reset();
	//iHTTPSessions.Reset();
	
	// connect to proxy if requested
	/*if (iConnectingProxy)
	{
		iConnectingProxy = EFalse;
		
		if (aResult)
		{
			iProxyConnector->StartL(EConProxy, &(iNetworkConnection->Connection()));
		//	ReportEventL(EKiEventProxyConnected);
		}		
		else
		{
		//	iTorrentMgr->SetAllTorrentsFailedL(_L("Failed to establish network connection."));
		//	ReportEventL(EKiEventProxyDisconnected);
		}
		
		//iTorrentMgr->NotifyEngineStateObserverL();
	}

	// start accepting incoming connections if requested
	if (iListenIssued)
	{
		iListenIssued = EFalse;
		
		if (aResult)
		{
			if (iSocketListener->StartL(EConListener, &(iNetworkConnection->Connection()), iLocalAddress) != KErrNone)
			{
				LWRITELN(LogL(), _L("Listening failed"));
				iIsListening = EFalse;
			}	
			else
			{
				LWRITELN(LogL(), _L("Listening"));
				ReportEventL(EKiEventListeningStarted);
			}	
		}			
		else
		{
			LWRITELN(LogL(), _L("Listening failed"));
			iIsListening = EFalse;
		}			
		
	//	iTorrentMgr->NotifyEngineStateObserverL();
	}*/
	

	//TODO always ask for IAP is not working
//	if (aResult && iAskForIapId)
//		iNetworkConnection->SetIapId(-1);
	
	// TODO Address
	/*TInetAddr address;
	if (Address(address) == KErrNone)
	{
		TBuf<128> addrBuf;
		address.Output(addrBuf);
		LWRITE(LogL(), _L("[NetworkManager] Local address: "));
		LWRITELN(LogL(), addrBuf);
	}*/
	
	//iStartingNetworkConnection = EFalse;
	
	HLWRITELN(LOG, _L("[CNetworkManager] OnNetworkConnectionStartedL end"));
}


EXPORT_C void CNetworkManager::Close(RHostResolver& aHostResolver)
{
	TInt index = -1;
	for (TInt i=0; i<iHostResolvers.Count(); i++)
		if (iHostResolvers[i]->iHostResolver == &aHostResolver)
			index = i;
	
	if (index >= 0)
	{
		delete iHostResolvers[index];
		iHostResolvers.Remove(index);
	}
	
	aHostResolver.Close();		
}


EXPORT_C void CNetworkManager::Close(RSocket& aSocket)
{
	TInt index = -1;
	for (TInt i=0; i<iSockets.Count(); i++)
		if (iSockets[i]->iSocket == &aSocket)
			index = i;
		
	if (index >= 0)
	{
		delete iSockets[index];
		iSockets.Remove(index);
	}
		
	aSocket.Close();	
}

EXPORT_C void CNetworkManager::Close(RHTTPSession& aSession)
{
	TInt index = -1;
	for (TInt i=0; i<iHTTPSessions.Count(); i++)
	if (iHTTPSessions[i]->iSession == &aSession)
		index = i;
		
	if (index >= 0)
	{
		delete iHTTPSessions[index];
		iHTTPSessions.Remove(index);
	}
	 
	aSession.Close();
}

/*void CNetworkManager::ReportEvent(TConnectorEvents aEvent)
{
	LWRITE(LogL(), _L("[NetworkManager] Proxy event: "));
	LWRITELN(LogL(), TInt(aEvent));
	
	switch (aEvent)
	{
		case ESucConnectedAndInitialized:
		{
			iLastProxyConnectionAttemptResult = EProxyConnectionSucceeded;
			
			for (TInt i=0; i<iProxyConnectionObservers.Count(); i++)
				iProxyConnectionObservers[i]->ReportProxyConnectionL(ETrue);
				
			iProxyConnectionObservers.Reset();
			
		//	iTorrentMgr->NotifyEngineStateObserverL();
			
			ReportEventL(EKiEventProxyConnected);
		}
		break;
		
		case EErrDisconnected:
		case EErrConnectionFailed:
		{
			//iTorrentMgr->NotifyEngineStateObserverL();
			ReportEventL(EKiEventProxyDisconnected);
		}
		break;
		
		default:
		break;
	}
	
	if ((aEvent == EErrConnectionFailed) || (aEvent == EErrDisconnected))
	{
		if (iLastProxyConnectionAttemptResult == EProxyConnectionPending)
			iLastProxyConnectionAttemptResult = EProxyConnectionFailed;
		
		for (TInt i=0; i<iProxyConnectionObservers.Count(); i++)
			iProxyConnectionObservers[i]->ReportProxyConnectionL(EFalse);
				
		iProxyConnectionObservers.Reset();
	}
}
 
void CNetworkManager::ReportMessage(const TDesC& aMsg)
{
	LWRITE(LogL(), _L("[NetworkManager] Proxy message: "));
	LWRITELN(LogL(), aMsg);	
}*/

void CNetworkManager::AcceptSocketL(RSocket* aSocket, TUint aPort, CNetworkConnection& aConnection)
{
	CleanupStack::PushL(aSocket);
	
	TInt connIndex = iNetworkConnections.Find(&aConnection);
	
	TInetAddr remoteAddress;
	(*aSocket).RemoteName(remoteAddress);
	TBuf<128> addrBuf;
	remoteAddress.Output(addrBuf);
	TBuf<250> info;
	info.Format(_L("[NetworkManager (NI:%d)] Accepting incoming connection from: %S"), connIndex, &addrBuf);	
	LWRITELN(LogL(), info);
	
	// TODO check local connection
	/*TInetAddr localAddress;
	if (IsListening() && (Address(localAddress) == KErrNone)) // only check if not connected through proxy
	{
		if (localAddress.Address() == remoteAddress.Address())
		{
			LWRITELN(LogL(), _L("[NetworkManager] Incoming connection from localhost, closing"));
			aInitializedSocket->Close();
			CleanupStack::PopAndDestroy(); // aInitializedSocket
			return;
		}
	}*/
	
	TInt socketAccepted = EFalse;
	for (TInt i=0; i<iSocketAccepters.Count(); i++)
	{
		if (iSocketAccepters[i]->AcceptSocketL(aSocket, aPort, aConnection, connIndex))
		{
			socketAccepted = ETrue;
			break;
		}
	}
	
	if (!socketAccepted)
	{
		aSocket->Close();
		delete aSocket;
	}	
	
	CleanupStack::Pop(); // aInitializedSocket
	
//	LWRITELN(LogL(), _L("[NetworkManager] Notifying observers"));
//	for (TInt i=0; i<iObservers.Count(); i++)
//		iObservers[i]->AcceptIncomingConnectionL(aInitializedSocket);
	//iTorrentMgr->RegisterIncomingConnectionL(aInitializedSocket);
}

/*EXPORT_C TInt CNetworkManager::Address(TInetAddr& aAddress)
{
	switch (iIncomingConnectionMode)
	{
		case EEnabledWithProxy:
		{
			if ((ProxyConnectorState() == EConnectedAndInitialized))
			{
				if (aAddress.Input(iProxyParameters->GetProxyAddress()) == KErrNone)
				{
					aAddress.SetPort(iProxyParameters->GetPartnerPort());
					
					return KErrNone;
				}
			}
			
			return KErrGeneral;
		}
		break;
		
		case EEnabledWithoutProxy:
		{
			if (IsListening() && (iLocalAddress.Address() != 0))
			{
				aAddress = iLocalAddress;
				return KErrNone;
			}
		}
		break;

		case EDisabled:
		default:
			break;
	}
		
	return KErrGeneral;
}*/


/*void CNetworkManager::SettingChangedL(TSymTorrentSetting aSetting)
{
	CSTPreferences* prefs = Preferences();

	if (aSetting == ESettingProxyConnectionPort)
	{
		if (iProxyParameters->GetConnectionPort() != prefs->ProxyConnectionPort())
		{
			iProxyParameters->SetConnectionPort(prefs->ProxyConnectionPort());
			
			if ((prefs->IncomingConnectionsMode() == EEnabledWithProxy) &&
				(ProxyConnectorState() != EConOffline))
			{
				DisconnectProxy();
				ConnectProxyL();
			}			
		}
	}
	else
	
	if (aSetting == ESettingProxyServicePort)
	{
		if (iProxyParameters->GetServicePort() != prefs->ProxyServicePort())
		{
			iProxyParameters->SetServicePort(prefs->ProxyServicePort());
			
			if ((prefs->IncomingConnectionsMode() == EEnabledWithProxy) &&
				(ProxyConnectorState() != EConOffline))
			{
				DisconnectProxy();
				ConnectProxyL();
			}			
		}
	}
	else
	
	if (aSetting == ESettingProxyHostName)
	{
		if (prefs->ProxyHostName() != iProxyParameters->GetProxyAddress())
		{
			iProxyParameters->SetProxyAddress(prefs->ProxyHostName());
			
			if ((prefs->IncomingConnectionsMode() == EEnabledWithProxy) &&
				(ProxyConnectorState() != EConOffline))
			{
				DisconnectProxy();
				ConnectProxyL();
			}
			
		}
		iProxyParameters->SetProxyAddress(prefs->ProxyHostName());	
	}
	else
	
	if (aSetting == ESettingIncomingPort)
	{
		if (iListenerParameters->GetConnectionPort() != prefs->IncomingPort())
		{
			iListenerParameters->SetConnectionPort(prefs->IncomingPort());
			
			if (IsListening())
			{
				StopListeningL();
				StartListeningL();
			}		
		}		
	}
	else
	
	if (aSetting == ESettingAccesPointId)
	{
		if (prefs->AccessPointId() > 0)
			SetNetworkConnectionId(prefs->AccessPointId());
	}
	else
	
	if (aSetting == ESettingIncomingConnectionsMode)
	{
		TIncomingConnectionsMode incomingConnectionsMode = 
			prefs->IncomingConnectionsMode();
			
		if (incomingConnectionsMode != EEnabledWithProxy)
		{
			if (ProxyConnectorState() != EConOffline)
				DisconnectProxy();
		}
			
		
		if (incomingConnectionsMode == EEnabledWithoutProxy)
			if (IsNetworkConnectionStarted()) 
				StartListeningL();
		else
			StopListeningL();
	}		
}*/

/*CSTPreferences* CNetworkManager::Preferences() 
{
	return iTorrentMgr->Preferences();
}*/

/*EXPORT_C TConnectorSimpleStates CNetworkManager::ProxyConnectorState() const {
	return iProxyConnector->GetConnectorState();
}*/

/*EXPORT_C void CNetworkManager::AddNetworkConnectionObserverL(MNetworkConnectionObserver* aObserver) 
{
	if (iNetworkConnectionObservers.Find(aObserver) < 0)
		User::LeaveIfError(iNetworkConnectionObservers.Append(aObserver));
}*/

EXPORT_C TBool CNetworkManager::IsNetworkConnectionStarted(TInt aIndex) const 
{
	#if defined(__WINS__) && !defined(EKA2)
		return ETrue;
	#else
		return iNetworkConnections[aIndex]->IsStarted();
	#endif
}

EXPORT_C RSocketServ& CNetworkManager::SocketServ() 
{
	return iSocketServ;
}

EXPORT_C void CNetworkManager::RemoveObserver(MKiNetworkManagerObserver* aObserver) 
{
	TInt index = iObservers.Find(aObserver);
	if (index >= 0)
		iObservers.Remove(index);
}

void CNetworkManager::ReportEventL(TKiNetworkManagerEvent aEvent)
{
	for (TInt i=0; i<iObservers.Count(); i++)
		iObservers[i]->NetworkManagerEventReceivedL(aEvent);
}

EXPORT_C void CNetworkManager::SetIncomingConnectionModeL(TIncomingConnectionsMode aIncomingConnectionMode) 
{	
	/*if (aIncomingConnectionMode != EEnabledWithProxy)
	{
		if (ProxyConnectorState() != EConOffline)
			DisconnectProxy();
	}
			
	if (aIncomingConnectionMode == EEnabledWithoutProxy)
	{
		if (IsNetworkConnectionStarted()) 
			StartListeningL();
		else
			StopListeningL();
	}*/
		
	iIncomingConnectionMode = aIncomingConnectionMode;
}

EXPORT_C void CNetworkManager::AddSocketAccepterL(MKiSocketAccepter* aSocketAccepter)
{
	iSocketAccepters.AppendL(aSocketAccepter);
}

EXPORT_C void CNetworkManager::RemoveSocketAccepter(MKiSocketAccepter* aSocketAccepter)
{
	TInt index = iSocketAccepters.Find(aSocketAccepter);
	if (index >= 0)
		iSocketAccepters.Remove(index);
}

EXPORT_C TBool CNetworkManager::IsListening(TInt aConnIndex, TUint aPort) const
{
	return iNetworkConnections[aConnIndex]->IsListening(aPort);
}

EXPORT_C void CNetworkManager::StopAllListening(TInt aConnIndex)
{
	iNetworkConnections[aConnIndex]->StopAllListening();
}

EXPORT_C TBool CNetworkManager::IsListening(TInt aConnIndex) const
{
	return iNetworkConnections[aConnIndex]->IsListening();
}

EXPORT_C TInt CNetworkManager::GetLocalAddress(TInt aConnIndex, TSockAddr& aAddr)
{
	CNetworkConnection& conn = *iNetworkConnections[aConnIndex];
	
	if (!conn.GotLocalAddress())
		return KErrGeneral;
	
	aAddr = conn.LocalAddress();
	return KErrNone;
}

EXPORT_C CBluetoothManager* CNetworkManager::GetBtManager()
{
	if (!iBtManager)
		iBtManager = CBluetoothManager::NewL();
	
	return iBtManager;
}

EXPORT_C void CNetworkManager::SetLog(CKiLogger* aLog) 
{
	iLog = aLog;
}

