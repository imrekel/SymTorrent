/*
 ============================================================================
 Name		: BluetoothDeviceDiscoverer.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CBluetoothDeviceDiscoverer implementation
 ============================================================================
 */

#include "BluetoothDeviceDiscoverer.h"
#include "BluetoothCommon.h"
#include "KiNetwork.pan"

//#include "NetworkManager.h"
//#include "KiLogger.h"
//#define LOG NETMGR->LogL()

CBluetoothDeviceDiscoverer::CBluetoothDeviceDiscoverer() :
	CActive(EPriorityStandard)
{
}

CBluetoothDeviceDiscoverer* CBluetoothDeviceDiscoverer::NewLC()
{
	CBluetoothDeviceDiscoverer* self = new ( ELeave ) CBluetoothDeviceDiscoverer();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CBluetoothDeviceDiscoverer* CBluetoothDeviceDiscoverer::NewL()
{
	CBluetoothDeviceDiscoverer* self = CBluetoothDeviceDiscoverer::NewLC();
	CleanupStack::Pop();
	return self;
}

void CBluetoothDeviceDiscoverer::ConstructL()
{
	CActiveScheduler::Add( this);
}

CBluetoothDeviceDiscoverer::~CBluetoothDeviceDiscoverer()
{
	Cancel();
	iNotifier.Close();
}

void CBluetoothDeviceDiscoverer::DoCancel()
{
	iNotifier.CancelNotifier(KDeviceSelectionNotifierUid);
	iNotifier.Close();
}

void CBluetoothDeviceDiscoverer::RunL()
{	
	iNotifier.CancelNotifier(KDeviceSelectionNotifierUid);
	iNotifier.Close();
	
	if (iStatus.Int() == KErrNone)
	{
	    if (iResultPckg().IsValidBDAddr())
	    {
	    	iObserver->OnBtDeviceDiscoveryCompleteL(iResultPckg().BDAddr());
	    	return;
	    }
	    	
	    iObserver->ReportBtDeviceDiscoveryErrorL(KErrNotFound);
	    return;
	}
	
	iObserver->ReportBtDeviceDiscoveryErrorL(iStatus.Int());
}

TInt CBluetoothDeviceDiscoverer::RunError(TInt aError)
{
	return aError;
}

void CBluetoothDeviceDiscoverer::StartDeviceDiscoveryL(MBluetoothDeviceDiscovererObserver* aObserver)
{
	if (IsActive())
		User::Panic(KLitKiNetworkPanic, KPanBtDeviceDiscovererIsAlreadyActive);
	
	iObserver = aObserver;
	
	User::LeaveIfError(iNotifier.Connect());

	TBTDeviceSelectionParams selectionFilter;
	TUUID targetServiceClass(KSdpServiceClassSerialPort);
	selectionFilter.SetUUID(KBtServiceUid);
	TBTDeviceSelectionParamsPckg pckg(selectionFilter);
	
	TBTDeviceResponseParams result;
	iResultPckg = TBTDeviceResponseParamsPckg(result);
	
	TRequestStatus status;
	iNotifier.StartNotifierAndGetResponse(iStatus, KDeviceSelectionNotifierUid, pckg, iResultPckg);
	//User::After(2000000);

	SetActive();
}

