/*
 ============================================================================
 Name		: BluetoothServiceDiscoverer.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CBluetoothServiceDiscoverer declaration
 ============================================================================
 */

#ifndef BLUETOOTHSERVICEDISCOVERER_H
#define BLUETOOTHSERVICEDISCOVERER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <btsdp.h>
#include <btmanclient.h>

// CLASS DECLARATION

class MBluetoothServiceDiscovererObserver
{
public:
	virtual void OnBtServiceDiscoveryCompleteL(TBTDevAddr aAddr, TInt aPort) = 0;
	virtual void ReportBtServiceDiscoveryErrorL(TBTDevAddr aAddr, TInt aError) = 0;
};

/**
 *  CBluetoothServiceDiscoverer
 * 
 */
class CBluetoothServiceDiscoverer : public CBase,
									public MSdpAgentNotifier
{
public:

	~CBluetoothServiceDiscoverer();

	static CBluetoothServiceDiscoverer* NewL();

	static CBluetoothServiceDiscoverer* NewLC();
	
public:
	
	inline void SetObserver(MBluetoothServiceDiscovererObserver* aObserver);
	
	/**
	 * Discovers service on the given device
	 */
	void DiscoverServiceL(const TBTDevAddr& aAddr);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CBluetoothServiceDiscoverer();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	
	void StartDiscoverL();
	
private: // from MSdpAgentNotifier
	
	void NextRecordRequestComplete(
			TInt aError,
			TSdpServRecordHandle aHandle,
			TInt aTotalRecordsCount);

		void AttributeRequestResult(
			TSdpServRecordHandle aHandle,
			TSdpAttributeID aAttrID,
			CSdpAttrValue* aAttrValue);

	    void AttributeRequestComplete(
			TSdpServRecordHandle aHandle,
			TInt aError);
	
private:
	
	RArray<TBTDevAddr> iDevicesToDiscover;
	
	MBluetoothServiceDiscovererObserver* iObserver;
	
//	RSdp iSdp;
//	
//	RSdpDatabase iSdpDatabase;
//	
//	TSdpServRecordHandle iSdpRecordHandle;
	
	CSdpAgent *iSdpAgent;
	
	TBool iGotService;
};

void CBluetoothServiceDiscoverer::SetObserver(MBluetoothServiceDiscovererObserver* aObserver) {
	iObserver = aObserver;
}

#endif // BLUETOOTHSERVICEDISCOVERER_H
