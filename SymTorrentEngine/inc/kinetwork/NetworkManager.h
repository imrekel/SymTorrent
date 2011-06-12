/**
 * ============================================================================
 *  Name     : CNetworkManager from NetworkManager.h
 *  Part of  : SymTorrent
 *  Created  : 10.04.2006 by Imre Kelényi
 * ============================================================================
 */

#ifndef NETWORKMANAGER_H__
#define NETWORKMANAGER_H__

// INCLUDES
#include <e32base.h>
#include <es_sock.h>
#include <in_sock.h>
#include <coemain.h>
#include <http\rhttpsession.h>
#include "WriteBuffer.h"
#include "NetworkConnection.h"

#ifdef EKA2
	#define KINETWORK_UID 0xA00036FC
#else
	#define KINETWORK_UID 0x2000CB8F
#endif

// FORWARD DECLARATIONS
class CSTTBaseData;
class CSTTConnectorProxy;
class CSTTConnectorListener;
class CSTTorrentManager;
class CSTPreferences;
class CKiLogger;
class CBluetoothManager;

const TUid KUidNetworkManagerSingleton = { 0x10000FF4 };

//#define KBT_serviceID 0xaf
//#define KBT_Channel   4

#define NETWORKMGR CNetworkManager::InstanceL()
#define NETMGR CNetworkManager::InstanceL()

enum TSTProxyConnectionAttemptResult
{
	EProxyConnectionSucceeded,
	EProxyConnectionFailed,
	EProxyConnectionPending
};

enum TKiSocketType
{
	EKiSocketConnected = 0,
	EKiSocketUDP
};


// FORWARD DECLARATIONS
class CNetworkConnection;

class MSocketOpenObserver {
public:
	virtual void SocketOpenedL(TBool aResult, RSocket& aSocket) = 0;
};

class MHostResolverOpenObserver {
public:
	virtual void HostResolverOpenedL(TBool aResult, RHostResolver& aHostResolver) = 0;
};

class MHTTPSessionOpenObserver {
public:
	virtual void HTTPSessionOpenedL(TBool aResult, RHTTPSession& aHTTPSession) = 0;
};

class MSTProxyConnectionObserver {
public:
	virtual void ReportProxyConnectionL(TBool aProxyConnectionSucceeded) = 0;
};

enum TKiNetworkManagerEvent
{
	//EKiEventProxyConnected = 0,
	//EKiEventProxyDisconnected,
	EKiEventListeningStarted,
	EKiEventListeningStopped
};

/**
 * MKiNetworkManagerObserver
 * 
 * Observs general events of the network manager
 */
class MKiNetworkManagerObserver
{
public:

//	virtual void AcceptIncomingConnectionL(RSocket* aInitializedSocket) = 0;
		
	virtual void NetworkManagerEventReceivedL(TKiNetworkManagerEvent aEvent) = 0;
	
	virtual void OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection, TInt aConnectionIndex) = 0;
		
	/**
	 * Called when an active connection is disconnected
	 */
	virtual void OnNetworkConnectionDownL(CNetworkConnection& aConnection, TInt aConnectionIndex) = 0;
};

/**
 * MKiSocketAccepter
 * 
 * Callback class for accepting incoming connections via sockets.
 */
class MKiSocketAccepter
{
public:
	
	/**
	 * Accepts an incoming socket. If the socket is accepted then its ownership
	 * must be taken and on cleanup it must be deleted from the heap.
	 * 
	 * Must return ETrue if the socket is accepted and EFalse if not.
	 */
	virtual TBool AcceptSocketL(RSocket* aSocket, TUint aPort, CNetworkConnection& aConnection, TInt aConnIndex) = 0;
};

/**
 * MKiAccessPointSupplier
 *
 * Interface used to query the access point id from the UI
 */
class MKiAccessPointSupplier
{
public:
	virtual TBool GetIapIdL(TInt32& aAccessPointId, TDes& aAccessPointName, TInt aNetConnIndex) = 0;

};
	
/**
 * CNetworkManager
 *
 * Singleton class, the instance can be accessed via the method InstanceL()
 */
class CNetworkManager : 	public CCoeStatic, 
						  	public MNetworkConnectionObserver,
						  	public MSocketListenerObserver
						  	
{		
	class TNetworkConnectionChunk
	{
	public:
		TNetworkConnectionChunk()
		 : iConnection(NULL)
		{}
		
		CNetworkConnection* iConnection;
	};
	
	class TSocketWithObserver : public TNetworkConnectionChunk
	{
	public:
		TSocketWithObserver() 
		 : iSocket(NULL), iOpenObserver(NULL), iSocketType(EKiSocketConnected)
		{}
		
		RSocket* iSocket;
		MSocketOpenObserver* iOpenObserver;
		TKiSocketType iSocketType;
	};
	
	class THostResolverWithObserver : public TNetworkConnectionChunk
	{
	public:
		THostResolverWithObserver() 
		 : iHostResolver(NULL), iOpenObserver(NULL)
		{}
		
		RHostResolver* iHostResolver;
		MHostResolverOpenObserver* iOpenObserver;
	};
	
	class THTTPSessionWithObserver : public TNetworkConnectionChunk
	{
	public:
		THTTPSessionWithObserver() 
		 : iSession(NULL), iOpenObserver(NULL)
		{}
		
		RHTTPSession* iSession;
		MHTTPSessionOpenObserver* iOpenObserver;
	};
	
	class TSocketListenerWithPort : public TNetworkConnectionChunk
	{
	public:
	
		TUint iPort;
	};
	
public: // exported functions
	
	enum TIncomingConnectionsMode
	{
		EEnabledWithProxy = 0,
		EEnabledWithoutProxy,
		EDisabled
	};

	IMPORT_C static CNetworkManager* InstanceL();
	
	/**
	 * Initializes the network manager with the given values. Should be called before
	 * the first use.
	 */
	IMPORT_C void Initialize(	/*TInt aProxyConnectionPort, 
						 		TInt aProxyServicePort,
						 		const TDesC& aProxyHostName,
						 		TInt aIncomingPort,
						 		TUint32 aDefaultAccesPointId,*/
						 		CKiLogger* aLog = NULL);
	
	/**
	 * Creates a new network connection for the given Iap ID.
	 * 
	 * @return the index of the newly created connection
	 */
	IMPORT_C TInt CreateNetworkConnectionL(	CNetworkConnection::TNetworkConnectionType aType = CNetworkConnection::ERConnectionBased, 
											TInt32 aIapId = -1);
						 		
	IMPORT_C void StartNetworkConnectionL(TInt aIndex = 0);
	
	IMPORT_C void CloseNetworkConnection(TInt aIndex = 0);
	
	
		
	IMPORT_C void OpenSocketL(RSocket& aSocket, TKiSocketType aSocketType, MSocketOpenObserver* aObserver, TInt aConnIndex);
	
	IMPORT_C void OpenConnectedSocketL(RSocket& aSocket, MSocketOpenObserver* aObserver, TInt aConnIndex);

	IMPORT_C void OpenUDPSocketL(RSocket& aSocket, MSocketOpenObserver* aObserver, TInt aConnIndex);
	
	IMPORT_C void OpenHostResolverL(RHostResolver& aHostResolver, MHostResolverOpenObserver* aObserver, TInt aConnIndex);
	
	IMPORT_C void OpenHTTPSessionL(RHTTPSession& aSession, MHTTPSessionOpenObserver* aObserver, TInt aConnIndex);
	
	// TODO: Remove all close methods
	IMPORT_C void Close(RHostResolver& aHostResolver);
	
	IMPORT_C void Close(RSocket& aSocket);
	
	IMPORT_C void Close(RHTTPSession& aSession);
	
	/**
	 * Connects to the proxy and starts accepting incoming connections
	 
	IMPORT_C void ConnectProxyL();
	
	IMPORT_C void DisconnectProxy();*/
	
	/**
	 * If the client is listening then it returns the local address of the device
	 * or the the adress on the proxy otherwise
	 * (it returns the address on which others can connect the device)
	 *
	 * @return KErrNone if no errors occured
	 */
//	IMPORT_C TInt Address(TInetAddr& aAddress);
	
	/**
	 * Starts listening and accepting incoming connections
	 */	
	IMPORT_C void StartListeningL(TInt aConnectionIndex, TUint aPort);

	IMPORT_C void StopListeningL(TInt aConnectionIndex, TUint aPort);
	
	IMPORT_C void StopAllListening(TInt aConnIndex);
	
//	IMPORT_C TConnectorSimpleStates ProxyConnectorState() const;
	
	//IMPORT_C void AddNetworkConnectionObserverL(MNetworkConnectionObserver* aObserver);
	
	IMPORT_C TBool IsNetworkConnectionStarted(TInt aConnIndex) const;
	
	IMPORT_C RSocketServ& SocketServ();
	
	/**
	 * @return the conection for the given index
	 */
	inline CNetworkConnection& Connection(TInt aIndex); 	// Should be removed after tracker has been fully integrated into the engine
	
	inline void AddObserverL(MKiNetworkManagerObserver* aObserver);
	
	IMPORT_C void RemoveObserver(MKiNetworkManagerObserver* aObserver);
	
	IMPORT_C void SetIncomingConnectionModeL(TIncomingConnectionsMode aIncomingConnectionMode);
	
	/**
	 * Sets whether the access point is always asked before establishing network connection
	 */
	inline void SetAskForAccessPoint(TBool aAsk);
	
	CKiLogger* LogL();
		
	IMPORT_C void SetLog(CKiLogger* aLog);
	
	IMPORT_C TInt GetLocalAddress(TInt aConnIndex, TSockAddr& aAddr);
	
	IMPORT_C CBluetoothManager* GetBtManager();

private:

	CNetworkManager(/*CSTTorrentManager* aTorrentMgr*/);

	void ConstructL();
	
public:
	
	inline void SetAccessPointSupplier(MKiAccessPointSupplier* aIapSupplier);
	
//	inline void SetNetworkConnectionId(TInt aIapId);

	~CNetworkManager();			
		
//	inline TBool IsIapSet() const;
		
	//inline void AddProxyConnectionObserverL(MSTProxyConnectionObserver* aObserver);		
	
	//inline TSTProxyConnectionAttemptResult LastProxyConnectionAttemptResult() const;	
		
	IMPORT_C TBool IsListening(TInt aConnIndex, TUint aPort) const;
	
	IMPORT_C TBool IsListening(TInt aConnIndex) const;
	
/*private: // from MSTTConnectorEvents

	void ReportEvent(TConnectorEvents aEvent);
	 
	void ReportMessage(const TDesC& aMsg); */
	
	IMPORT_C void AddSocketAccepterL(MKiSocketAccepter* aSocketAccepter);
	
	IMPORT_C void RemoveSocketAccepter(MKiSocketAccepter* aSocketAccepter);
	
private: // from MSocketListenerObserver

	void AcceptSocketL(RSocket* aSocket, TUint aPort, CNetworkConnection& aConnection);

private: // from MNetworkConnectionObserver

	void OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& aConnection);
	
	void OnNetworkConnectionDownL(CNetworkConnection& aConnection);
	
//private:

//	void SettingChangedL(TSymTorrentSetting aSetting);
	
private:
	
//	CSTPreferences* Preferences();
	
	/**
	 * Called by timer periodically
	 */
	void OnTimerL();
	
//	TBool GetIapIdL(CNetworkConnection& aConnection);
	
	void SetHTTPSessionInfoL(RHTTPSession& aSession, CNetworkConnection& aNetworkConnection);
	
	/**
	 * Reports an event to the observers.
	 */
	void ReportEventL(TKiNetworkManagerEvent aEvent);
	
	void DoOpenSocketL(RSocket& aSocket, TKiSocketType aSocketType, CNetworkConnection& aConnection, MSocketOpenObserver* aObserver);

private:

	//CSTTConnector*				iProxyConnector;
	
	//CSTTBaseData*				iProxyParameters;
					
	//CSTTConnector*				iSocketListener;
			
	//CSTTBaseData*				iListenerParameters;
	
	//RPointerArray<CSocketListenerBase> iSocketListeners;
	
	//TBool						iConnectingProxy;

	CKiLogger*					iLog;

	//TUint						iEllapsedTime;
	
	//TUint						iIdleTime;
	
	RPointerArray<CNetworkConnection> iNetworkConnections;
	
	RSocketServ					iSocketServ;
	
	//TBool						iConnectionActive;
	
	MKiAccessPointSupplier*		iAccessPointSupplier;
	
	RPointerArray<TSocketWithObserver> 		iSockets;
	
	RPointerArray<THostResolverWithObserver> 	iHostResolvers;
	
	RPointerArray<THTTPSessionWithObserver> 	iHTTPSessions;
	
	RPointerArray<TSocketListenerWithPort>		iListeners;
	
//	RPointerArray<MSTProxyConnectionObserver> iProxyConnectionObservers;
	
//	RPointerArray<MNetworkConnectionObserver> iNetworkConnectionObservers;
	
	//TInetAddr					iLocalAddress;
	
	//TBool						iIsListening;
	
	//TBool						iListenIssued;
	
	//TSTProxyConnectionAttemptResult	iLastProxyConnectionAttemptResult;
	
	//CSTTorrentManager* 			iTorrentMgr;
	
	//TBool 						iStartingNetworkConnection;
	
	RPointerArray<MKiNetworkManagerObserver> 	iObservers;
	
	TIncomingConnectionsMode 					iIncomingConnectionMode;
	
	TBool 										iAskForIapId;
	
	RPointerArray<MKiSocketAccepter>	 		iSocketAccepters;
	
	CBluetoothManager*							iBtManager;
};	

inline void CNetworkManager::SetAccessPointSupplier(MKiAccessPointSupplier* aIapSupplier) {
	iAccessPointSupplier = aIapSupplier;
}

/*inline TBool CNetworkManager::IsIapSet() const {
	return iNetworkConnection->IsIapSet();
}

inline void CNetworkManager::SetNetworkConnectionId(TInt aIapId) {
	iNetworkConnection->SetIapId(aIapId);
}*/

/*inline void CNetworkManager::AddProxyConnectionObserverL(MSTProxyConnectionObserver* aObserver) {
	if (iProxyConnectionObservers.Find(aObserver) < 0)
		User::LeaveIfError(iProxyConnectionObservers.Append(aObserver));
}

inline TSTProxyConnectionAttemptResult CNetworkManager::LastProxyConnectionAttemptResult() const {
	return iLastProxyConnectionAttemptResult;
}

inline TBool CNetworkManager::IsListening() const {
	return iIsListening;
}

inline CNetworkConnection& CNetworkManager::Connection(TInt aIndex) {
	return *iNetworkConnections[aIndex];
}

inline CSTTBaseData* CNetworkManager::ProxyParameters() const {
	return iProxyParameters;
}

inline CSTTBaseData* CNetworkManager::ListenerParameters() const {
	return iListenerParameters;
}*/

inline void CNetworkManager::AddObserverL(MKiNetworkManagerObserver* aObserver) {
	User::LeaveIfError(iObservers.Append(aObserver));
}
	
inline void CNetworkManager::SetAskForAccessPoint(TBool aAsk) {
	iAskForIapId = aAsk;
}

inline CNetworkConnection& CNetworkManager::Connection(TInt aIndex) {
	return *iNetworkConnections[aIndex];
}

inline CNetworkManager& NetMgr() { return *CNetworkManager::InstanceL(); }

#endif

// End of File
