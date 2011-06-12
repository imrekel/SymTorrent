/*
 ============================================================================
 Name		: BluetoothManager.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CBluetoothManager implementation
 ============================================================================
 */

#include "BluetoothManager.h"
#include "BluetoothServiceDiscoverer.h"
#include "BluetoothDeviceDiscoverer.h"

//#include "NetworkManager.h"
//#include "KiLogger.h"
//#define LOG NETMGR->LogL()

CBluetoothManager::CBluetoothManager()
{
	// No implementation required
}

CBluetoothManager::~CBluetoothManager()
{
	iServiceDiscObservers.Reset();
	delete iDeviceDiscoverer;
	delete iServiceDiscoverer;
}

CBluetoothManager* CBluetoothManager::NewLC()
{
	CBluetoothManager* self = new (ELeave)CBluetoothManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CBluetoothManager* CBluetoothManager::NewL()
{
	CBluetoothManager* self=CBluetoothManager::NewLC();
	CleanupStack::Pop(); // self;
	return self;
}

void CBluetoothManager::ConstructL()
{

}

void CBluetoothManager::ContinueDeviceDiscoveryL()
{
	TBool hasDiscoveryRequest = EFalse;
	
	for (TInt i=0; i<iServiceDiscObservers.Count(); i++)
	{
		if (iServiceDiscObservers[i].iAddr == 0)
		{
			iDeviceDiscoverer->StartDeviceDiscoveryL(this);
			hasDiscoveryRequest = ETrue;
			break;
		}
	}
	
	if (!hasDiscoveryRequest)
	{
		delete iDeviceDiscoverer;
		iDeviceDiscoverer = NULL;
	}
}

EXPORT_C void CBluetoothManager::DiscoverBluetoothDeviceL(MBluetoothDiscoveryObserver* aObserver)
{
	iServiceDiscObservers.AppendL(TBtAddrWithObserver(aObserver, 0));
	
	if (!iDeviceDiscoverer)
	{
		iDeviceDiscoverer = CBluetoothDeviceDiscoverer::NewL();
		iDeviceDiscoverer->StartDeviceDiscoveryL(this);
	}
}

void CBluetoothManager::OnBtServiceDiscoveryCompleteL(TBTDevAddr aAddr, TInt aPort)
{
	for (TInt i=0; i<iServiceDiscObservers.Count(); i++)
	{
		if (iServiceDiscObservers[i].iAddr == aAddr)
		{
			iServiceDiscObservers[i].iObserver->OnBtDeviceDiscoveredL(aAddr, aPort);
			iServiceDiscObservers.Remove(i);
			break;
		}
	}
}

void CBluetoothManager::ReportBtServiceDiscoveryErrorL(TBTDevAddr aAddr, TInt /*aError*/)
{
	for (TInt i=0; i<iServiceDiscObservers.Count(); i++)
	{
		if (iServiceDiscObservers[i].iAddr == aAddr)
		{
			iServiceDiscObservers[i].iObserver->OnBtDeviceDiscoveryFailedL();
			iServiceDiscObservers.Remove(i);
			break;
		}
	}
}

void CBluetoothManager::OnBtDeviceDiscoveryCompleteL(TBTDevAddr aAddr)
{
	for (TInt i=0; i<iServiceDiscObservers.Count(); i++)
	{
		if (iServiceDiscObservers[i].iAddr == 0)
		{
			iServiceDiscObservers[i].iAddr = aAddr;
			
			if (!iServiceDiscoverer)
			{
				iServiceDiscoverer = CBluetoothServiceDiscoverer::NewL();
				iServiceDiscoverer->SetObserver(this);
			}
			
			iServiceDiscoverer->DiscoverServiceL(aAddr);
			
			break;
		}
	}
	
	ContinueDeviceDiscoveryL();
}

void CBluetoothManager::ReportBtDeviceDiscoveryErrorL(TInt /*aError*/)
{
	for (TInt i=0; i<iServiceDiscObservers.Count(); i++)
	{
		if (iServiceDiscObservers[i].iAddr == 0)
		{
			iServiceDiscObservers[i].iObserver->OnBtDeviceDiscoveryFailedL();
			iServiceDiscObservers.Remove(i);
			break;
		}
	}
	
	ContinueDeviceDiscoveryL();
}
	
