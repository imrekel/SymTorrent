/*
 ============================================================================
 Name		: BluetoothManager.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CBluetoothManager declaration
 ============================================================================
 */

#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "BluetoothServiceDiscoverer.h"
#include "BluetoothDeviceDiscoverer.h"

class CBluetoothServiceDiscoverer;
class CBluetoothDeviceDiscoverer;

// CLASS DECLARATION

class MBluetoothDiscoveryObserver
{
public:
	virtual void OnBtDeviceDiscoveredL(const TBTDevAddr& aAddr, TInt aChanel) = 0;
	virtual void OnBtDeviceDiscoveryFailedL() = 0;
};

/**
 *  CBluetoothManager
 */
class CBluetoothManager : 	public CBase, 
							public MBluetoothServiceDiscovererObserver,
							public MBluetoothDeviceDiscovererObserver
{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CBluetoothManager();

	/**
	 * Two-phased constructor.
	 */
	static CBluetoothManager* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CBluetoothManager* NewLC();
	
public: // new methods
	
	IMPORT_C void DiscoverBluetoothDeviceL(MBluetoothDiscoveryObserver* aObserver);

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CBluetoothManager();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	
	void ContinueDeviceDiscoveryL();
	
private: // from MBluetoothServiceDiscovererObserver
	
	void OnBtServiceDiscoveryCompleteL(TBTDevAddr aAddr, TInt aPort);
	void ReportBtServiceDiscoveryErrorL(TBTDevAddr aAddr, TInt aError);
	
private: // from MBluetoothDeviceDiscovererObserver
	
	void OnBtDeviceDiscoveryCompleteL(TBTDevAddr aAddr);
	void ReportBtDeviceDiscoveryErrorL(TInt aError);
	
private:
	
	class TBtAddrWithObserver
	{
	public:
		TBtAddrWithObserver(MBluetoothDiscoveryObserver* aObserver, const TBTDevAddr& aAddr)
		 : iObserver(aObserver), iAddr(aAddr) {}
	public:
		MBluetoothDiscoveryObserver* iObserver;
		TBTDevAddr iAddr;
	};
	
	RArray<TBtAddrWithObserver> iServiceDiscObservers;
	
	CBluetoothServiceDiscoverer* iServiceDiscoverer;
	CBluetoothDeviceDiscoverer* iDeviceDiscoverer;

};

#endif // BLUETOOTHMANAGER_H
