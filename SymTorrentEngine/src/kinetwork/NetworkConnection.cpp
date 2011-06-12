#include "NetworkConnection.h"
#include "KiNetwork.pan"
#include "NetworkManager.h"
#include "TCPSocketListener.h"
#include "BluetoothSocketListener.h"
#include "KiNetworkLog.h"
#include <CommDbConnPref.h>
#include <btnotifierAPI.h>

CNetworkConnection::CNetworkConnection(RSocketServ& aSocketServer, CNetworkManager& aNetMgr)
 : CActive(EPriorityStandard), iIapId(-1), iSocketServer(aSocketServer), iState(ENCUninitialized), iGotLocalAddress(EFalse), iNetMgr(aNetMgr)
{ 	
}


void CNetworkConnection::ConstructL()
{
	CActiveScheduler::Add(this);
}

CNetworkConnection::~CNetworkConnection()
{
	iSocketListeners.ResetAndDestroy();
	
	Cancel();
	
	if (iType != EBluetooth)
		iConnection.Close();
	else
		iBluetoothPowerNotifier.Close();
}

EXPORT_C TBool CNetworkConnection::GetIapAndInitializeL(TInt aConnectionId, MKiAccessPointSupplier* aAccessPointSupplier)
{					
	if (iState == ENCUninitialized)
	{
		iState = ENCInitializing;
		TInt32 iap;
		TBuf<150> iapName;
		if (aAccessPointSupplier->GetIapIdL(iap, iapName, aConnectionId))
		{
			if (iap == KBluetoothIapId)
			{
				InitializeL(EBluetooth, iap);
			}
			else
				InitializeL(ERConnectionBased, iap);
			
			return ETrue;
		}
		
		iState = ENCUninitialized;					
		return EFalse;		
	}
	
	return EFalse;	
}

EXPORT_C void CNetworkConnection::InitializeL(TNetworkConnectionType aType, TInt aIapId)
{
	if ((iState != ENCUninitialized) && (iState != ENCInitializing))
		User::Panic(KLitKiNetworkPanic, KPanNetworkConnectionIsAlreadyInitialized);
	
	iType = aType;
	iIapId = aIapId;
	
	iState = ENCStopped;
}

EXPORT_C void CNetworkConnection::SetUninitialized()
{
	Close();
	iSocketListeners.ResetAndDestroy();
	
	iState = ENCUninitialized;
}

void CNetworkConnection::StartL()
{
	if (iState == ENCUninitialized) 
		User::Panic(KLitKiNetworkPanic, KPanNetworkConenctionIsNotInitialized);
	
	if ((iState == ENCStopped) && (!IsActive()))
	{
		Close();
		
		if (iType == EBluetooth)
		{		
			iBluetoothPowerNotifier.Close();			
			iBluetoothPowerNotifier.Connect();
			
			TPckgBuf<TBool> dummy;
			TPckgBuf<TBool> result;
	
			iState = ENCStarting;
			
			iBluetoothPowerNotifier.StartNotifierAndGetResponse(
				iStatus,
				KPowerModeSettingNotifierUid,
				iBtPowerNotifierDummy,
				iBtPowerNotifierResult
				);
			
			SetActive();				
		}
		else
		{
			TCommDbConnPref prefs;
			prefs.SetIapId(iIapId);
			prefs.SetDirection(ECommDbConnectionDirectionOutgoing);
			prefs.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
			prefs.SetBearerSet(KCommDbBearerUnknown);

			User::LeaveIfError(iConnection.Open(iSocketServer));
			
			iState = ENCStarting;
			iConnection.Start(prefs, iStatus);
			SetActive();						
		}
	}
}

		
void CNetworkConnection::Close()
{
	//LOG->WriteLineL(_L("[Network] Network connection CLOSED"));
	Cancel();
	
	if (iType != EBluetooth)
		iConnection.Close();
	else
		iBluetoothPowerNotifier.Close();
	
	iState = ENCStopped;
	iGotLocalAddress = EFalse;
}


void CNetworkConnection::DoCancel()
{
	if (iType == ERConnectionBased)
		iConnection.Close();
	else
		if (iState == ENCStarting)
			iBluetoothPowerNotifier.CancelNotifier(KPowerModeSettingNotifierUid);		

	iState = ENCStopped;
}


void CNetworkConnection::RunL()
{	
	if (iState == ENCStarting) // starting connection
	{
		if (iStatus.Int() == KErrNone)
	    {	    					
			switch (iType)
			{
				case EBluetooth:
				{
					if(!iBtPowerNotifierResult()) // failed to start Bluetooth
					{
						LWRITELN(iNetMgr.LogL(), _L("[Network] Failed to start Bluetooth"));
						
						Close();
						if (iNetworkConnectionObserver)
							iNetworkConnectionObserver->OnNetworkConnectionStartedL(EFalse, *this);
						
						return;
					}
								
					iState = ENCStarted;
					iBluetoothPowerNotifier.Close();
				}
				break;
				
				case ERConnectionBased:
				{
					iState = ENCStarted;
							
					TInetAddr addr;
					TInt res = GetLocalIPAddressL(addr);
					
					if (res == KErrNone)
					{
						iLocalAddress = addr;
						iGotLocalAddress = ETrue;
					}	
				}
				break;
			}
						
			if (iNetworkConnectionObserver)
				iNetworkConnectionObserver->OnNetworkConnectionStartedL(ETrue, *this);
			
			if (iType == ERConnectionBased)
				ScheduleProgressNotification();						
	    }
	    else
	    {
	    	Close();
		//	LOG->WriteLineL(_L("[Network] Starting RConnection failed!"));
			
	    	if (iNetworkConnectionObserver)
	    		iNetworkConnectionObserver->OnNetworkConnectionStartedL(EFalse, *this);	    	
	    }		
	}
	else // progress notification
	{
		if (iStatus == KErrNone)
		{
			//if (iProgress.iStage==KLinkLayerOpen); // interface up
			if (iProgress().iStage == KConnectionUninitialised) // interface down
			{
				//LOG->WriteLineL(_L("[Network] RConnection down!"));
				Close();
				
				if (iNetworkConnectionObserver)
					iNetworkConnectionObserver->OnNetworkConnectionDownL(*this);				
			}
			
			if (iState == ENCStarted)
				ScheduleProgressNotification();
		}
		else
		{
			//TODO failed to get progress information
		}		
	}       
}

void CNetworkConnection::ScheduleProgressNotification()
{
	if (IsActive()) return;

	// ...

	iConnection.ProgressNotification(iProgress, iStatus);
	SetActive(); 
}

CSocketListenerBase* CNetworkConnection::GetListener(TUint aPort)
{
	for (TInt i=0; i<iSocketListeners.Count(); i++)
		if (iSocketListeners[i]->Port() == aPort)
			return iSocketListeners[i];
	
	return NULL;
}

const CSocketListenerBase* CNetworkConnection::GetListener(TUint aPort) const
{
	return const_cast<CNetworkConnection*>(this)->GetListener(aPort);
}

TInt CNetworkConnection::StartListeningL(TUint aPort, MSocketListenerObserver* aObserver)
{
	HLWRITELN(iNetMgr.LogL(), _L("[CNetworkConnection] StartListeningL start"));
	
	if (iState != ENCStarted) 
		User::Panic(KLitKiNetworkPanic, KPanFailedToStartListeningNetworkConnectionNotStarted);
	
	CSocketListenerBase* listener = GetListener(aPort);
	
	if (!listener)
	{
		if (iType == EBluetooth)
		{
			listener = CBluetoothSocketListener::NewLC(iSocketServer, *this);
			iSocketListeners.AppendL(listener);
			CleanupStack::Pop();
			
			HLWRITELN(iNetMgr.LogL(), _L("Bluetooth listener created"));
		}
		else
		{
			listener = CTCPSocketListener::NewLC(iSocketServer, *this);
			iSocketListeners.AppendL(listener);
			CleanupStack::Pop();
			
			HLWRITELN(iNetMgr.LogL(), _L("TCP listener created"));
		}
	}
	
	TInt res = KErrNone;
	if (!listener->IsListening())
	{
		res = listener->StartListeningL(aPort);
		if (res != KErrNone)
		{
			LWRITE(iNetMgr.LogL(), _L("[NetworkConnection] Starting listening failed - PORT: "));
			LWRITELN(iNetMgr.LogL(), aPort);						
			
			delete listener;
			iSocketListeners.Remove(iSocketListeners.Count() - 1);
		}
		else
		{
			LWRITE(iNetMgr.LogL(), _L("[NetworkConnection] Listening started - PORT: "));
			LWRITELN(iNetMgr.LogL(), aPort);
			
			listener->SetObserver(aObserver);
			
			TInetAddr addr;
			if (iType == ERConnectionBased)
				GetLocalIPAddressL(addr);
			addr.SetPort(aPort);			
			iLocalAddress = addr;
			iGotLocalAddress = ETrue;
		}		
	}
	
	return res;
}
	
void CNetworkConnection::StopListening(TUint aPort)
{
	for (TInt i=0; i<iSocketListeners.Count(); i++)
	{
		if (iSocketListeners[i]->Port() == aPort)
		{
			delete iSocketListeners[i];
			iSocketListeners.Remove(i);
		}
	}		
}

void CNetworkConnection::StopAllListening()
{
	iSocketListeners.ResetAndDestroy();
}

TBool CNetworkConnection::IsListening(TUint aPort) const
{
	const CSocketListenerBase* listener = GetListener(aPort);
	if (listener && (listener->IsListening()))
		return ETrue;
	
	return EFalse;
}

TBool CNetworkConnection::IsListening() const
{
	for (TInt i=0; i<iSocketListeners.Count(); i++)
		if (iSocketListeners[i]->IsListening())
			return ETrue;
	
	return EFalse;
}

TInt CNetworkConnection::GetLocalIPAddressL(TInetAddr& aAddr)
{
	RSocket sock;
	User::LeaveIfError(sock.Open(iSocketServer, KAfInet, KSockStream, KProtocolInetTcp, BaseConnection()));
	 
	// Get the IAP id of the underlying interface of this RConnection
	TUint32 iapId = 0;
	User::LeaveIfError(BaseConnection().GetIntSetting(_L("IAP\\Id"), iapId));
	
	// Get IP information from the socket
	TSoInetInterfaceInfo ifinfo;
	TPckg<TSoInetInterfaceInfo> ifinfopkg(ifinfo);
	 
	TSoInetIfQuery ifquery;
	TPckg<TSoInetIfQuery> ifquerypkg(ifquery);
	 
	// To find out which interfaces are using our current IAP, we must
	// enumerate and go through all of them and make a query by name
	// for each.
	User::LeaveIfError(sock.SetOpt(KSoInetEnumInterfaces, KSolInetIfCtrl));
	while(sock.GetOpt(KSoInetNextInterface, KSolInetIfCtrl, ifinfopkg) == KErrNone)
	{
		ifquery.iName = ifinfo.iName;
		TInt err = sock.GetOpt(KSoInetIfQueryByName, KSolInetIfQuery, ifquerypkg);
	 
		if (err == KErrNone && ifquery.iZone[1] == iapId) // IAP ID is index 1 of iZone
		{
	 		// We have found an interface using the IAP we are interested in.
	    	TInetAddr ad = (TInetAddr&)ifinfo.iAddress;
			// must filter 0 and addresses 169.254.x.x (fake responses)
			if ( (ad.Address() != 0)&& ((ad.Address() & 0xFFFF0000) != 0xA9FE0000) )
			{
	    		// found a IPv4 address
	    		aAddr = ad;
	 
	    		sock.Close();
	    		return err; // stop & return KErrNone
	   		}
	  	}
	  	else if(err != KErrNone)
	  	{
	   		sock.Close();
	   		return err; // return with error
	  	}
	} // while
	 
	sock.Close();
	return KErrNotFound; // return with KErrNotFound
}

