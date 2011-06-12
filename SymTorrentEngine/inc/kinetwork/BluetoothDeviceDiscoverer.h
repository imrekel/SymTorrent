/*
 ============================================================================
 Name		: BluetoothDeviceDiscoverer.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CBluetoothDeviceDiscoverer declaration
 ============================================================================
 */

#ifndef BLUETOOTHDEVICEDISCOVERER_H
#define BLUETOOTHDEVICEDISCOVERER_H

#include <bt_sock.h>
#include <BTExtNotifiers.h>

/**
 * MBluetoothDeviceDiscovererObserver
 */
class MBluetoothDeviceDiscovererObserver
{
public:
	virtual void OnBtDeviceDiscoveryCompleteL(TBTDevAddr aAddr) = 0;
	virtual void ReportBtDeviceDiscoveryErrorL(TInt aError) = 0;
};

/**
 * CBluetoothDeviceDiscoverer
 */
class CBluetoothDeviceDiscoverer : public CActive
{
public:
	// Cancel and destroy
	~CBluetoothDeviceDiscoverer();

	// Two-phased constructor.
	static CBluetoothDeviceDiscoverer* NewL();

	// Two-phased constructor.
	static CBluetoothDeviceDiscoverer* NewLC();
	
	void StartDeviceDiscoveryL(MBluetoothDeviceDiscovererObserver* aObserver);

public:
	// New functions
	// Function for making the initial request
	void StartL(TTimeIntervalMicroSeconds32 aDelay);

private:
	// C++ constructor
	CBluetoothDeviceDiscoverer();

	// Second-phase constructor
	void ConstructL();

private:
	
	void RunL();

	void DoCancel();

	TInt RunError(TInt aError);

private:
	MBluetoothDeviceDiscovererObserver* iObserver;
	
	RNotifier iNotifier;
	
	TBTDeviceResponseParamsPckg iResultPckg;
};

#endif // BLUETOOTHDEVICEDISCOVERER_H
