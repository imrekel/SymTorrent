/*
 ============================================================================
 Name		: BluetoothSocketListener.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CBluetoothSocketListener implementation
 ============================================================================
 */

#include "BluetoothSocketListener.h"
#include "BluetoothCommon.h"
#include "NetworkManager.h"
#include "KiNetwork.pan"

#include <es_sock.h>
#include <btdevice.h>
#include <bt_sock.h>
#include <btmanclient.h>

_LIT(KBtServiceDescFormat, "Channel: %d");

CBluetoothSocketListener::CBluetoothSocketListener(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection)
 : CSocketListenerBase(aSocketServ, aNetworkConnection)
{
}

CBluetoothSocketListener::~CBluetoothSocketListener()
{
	StopListeningL();
	TRAPD(err, StopServiceAdvertiserL());
}

CBluetoothSocketListener* CBluetoothSocketListener::NewLC(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection)
{
	CBluetoothSocketListener* self = new (ELeave)CBluetoothSocketListener(aSocketServ, aNetworkConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CBluetoothSocketListener* CBluetoothSocketListener::NewL(RSocketServ& aSocketServ, CNetworkConnection& aNetworkConnection)
{
	CBluetoothSocketListener* self = CBluetoothSocketListener::NewLC(aSocketServ, aNetworkConnection);
	CleanupStack::Pop(); // self;
	return self;
}

void CBluetoothSocketListener::ConstructL()
{
	BaseConstructL();
}

TInt CBluetoothSocketListener::StartListeningL(TUint /*aPort*/)
{
	if (iState != ENotListening)
		User::Panic(KLitKiNetworkPanic, KPanSocketListenerIsAlreadyListening);
	
	
	
	// load protocol, L2CAP
	//   TProtocolDesc pdesc;
	// User::LeaveIfError(iSocketServ.FindProtocol(_L("RFCOMM"), pdesc));

    // open a socket
    User::LeaveIfError(iSocketListener.Open(iSocketServ, KBTAddrFamily, KSockStream, KRFCOMM));
    
    TInt channel;
    iSocketListener.GetOpt(KRFCOMMGetAvailableServerChannel, KSolBtRFCOMM, channel);
    iPort = (TUint)channel;

    // bluetooth socket address object
    TBTSockAddr btsockaddr;
    btsockaddr.SetPort(iPort);
         
    // bind socket
    User::LeaveIfError(iSocketListener.Bind(btsockaddr));
    // listen on port
    TInt err = iSocketListener.Listen(5);
    if (err != KErrNone)
    	return err;
    
    // set channel security
    SetSecurityL(btsockaddr, iPort);   
    
    delete iBlankSocket;
	iBlankSocket = NULL;
	
	iBlankSocket = new RSocket();
	iBlankSocket->Open(iSocketServ);
	iSocketListener.Accept(*iBlankSocket, iStatus);
	
	StartServiceAdvertiserL(iPort);
	
	iState = EListening;
	SetActive();
	
	/*User::LeaveIfError(iSocketListener.Open(iSocketServ, KAfInet, KSockStream, 
		KProtocolInetTcp, iNetworkConnection.BaseConnection()));
		
	TInetAddr addr;
	addr.SetPort(iPort);
	
	TInt err = iSocketListener.Bind(addr);				
	if (err != KErrNone)
		return err;
	
	err = iSocketListener.Listen(5);
	if (err != KErrNone)
		return err;
	
	delete iBlankSocket;
	iBlankSocket = NULL;
	
	iBlankSocket = new RSocket();
	iBlankSocket->Open(iSocketServ);
	iSocketListener.Accept(*iBlankSocket, iStatus); 
	iState = EListening;
	SetActive();*/
	
	return KErrNone;
}

void CBluetoothSocketListener::SetSecurityL(TBTSockAddr& aAddr, TInt /*aPort*/)
{
	TBTServiceSecurity secSettings;
	TUid settingsUID;
	settingsUID.iUid = KBtServiceUid;
	secSettings.SetUid(settingsUID);
	secSettings.SetAuthentication(EFalse);
	secSettings.SetAuthorisation(EFalse);
	secSettings.SetEncryption(EFalse);
 
    aAddr.SetSecurity(secSettings);
}
	
void CBluetoothSocketListener::StopListeningL()
{
	Cancel();
	iSocketListener.Close();
	delete iBlankSocket;
	iBlankSocket = NULL;
	
	UpdateServiceAvailabilityL(EFalse);

	iState = ENotListening;
}

void CBluetoothSocketListener::RunL() 
{
	if (iState == EListening)
	{
		if (iStatus.Int() == KErrNone)
		{
			if (iObserver)
				iObserver->AcceptSocketL(iBlankSocket, iPort, iNetworkConnection);
			else
				delete iBlankSocket;

			iBlankSocket = NULL;
			iBlankSocket = new RSocket();
			iBlankSocket->Open(iSocketServ);
			iSocketListener.Accept(*iBlankSocket, iStatus); 
			SetActive();
		}
		else
			StopListeningL();
	}
}
	
void CBluetoothSocketListener::DoCancel()
{
	iSocketListener.CancelAll();
}

void CBluetoothSocketListener::StartServiceAdvertiserL(TInt aChannel)
{
	if (iServiceRecord != 0)
	{
		UpdateServiceAvailabilityL(ETrue);
		return;
	}		
	
    // open sdp session
    User::LeaveIfError(iSdp.Connect());
    // open sdp database session
    User::LeaveIfError(iSdpDB.Open(iSdp));
    // create a record of the correct service class
 //   TUUID serviceUUID(KBtServiceUid);
    iSdpDB.CreateServiceRecordL(KSdpServiceClassSerialPort, iServiceRecord);

    // add a protocol to the record
    CSdpAttrValueDES* protocolDescriptorList = CSdpAttrValueDES::NewDESL(NULL);
    CleanupStack::PushL(protocolDescriptorList);

    TBuf8<1> channel;    
    channel.Append((TChar)aChannel);
    
    // create protocol list for our service
    protocolDescriptorList
    ->StartListL()   //  list of protocols required for this method
    	->BuildDESL()
            ->StartListL()
                ->BuildUUIDL(KL2CAP)
            ->EndListL()
        ->BuildDESL()
            ->StartListL()
                ->BuildUUIDL(KRFCOMM)
                ->BuildUintL(channel)
            ->EndListL()     
    ->EndListL();

    // set protocol list to the record
    iSdpDB.UpdateAttributeL(iServiceRecord, KSdpAttrIdProtocolDescriptorList,
        *protocolDescriptorList);
    CleanupStack::PopAndDestroy(protocolDescriptorList);
    
    iSdpDB.UpdateAttributeL(iServiceRecord, KSdpAttrIdServiceID, KBtServiceUid);

    // add a name to the record
    iSdpDB.UpdateAttributeL(iServiceRecord,
        KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceName, KBTServiceName);
  
    // add a description to the record
    TBuf<30> serviceDesc;
    serviceDesc.Format(KBtServiceDescFormat, aChannel);
    iSdpDB.UpdateAttributeL(iServiceRecord, 
    	KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceDescription, serviceDesc);

    // set service available
    UpdateServiceAvailabilityL(ETrue);
}

void CBluetoothSocketListener::StopServiceAdvertiserL()
{
	if ( iServiceRecord != 0 )
    {
	    // delete out record from service discovery database
	    iSdpDB.DeleteRecordL(iServiceRecord);
	    // close sdp and sdp db sessions
	    iSdpDB.Close();
	    iSdp.Close();
	    iServiceRecord = 0;
    }
}

void CBluetoothSocketListener::UpdateServiceAvailabilityL(TBool aAvailable)
{
	TInt state = aAvailable ? 0xFF : 0x00;
	// set availability
	iSdpDB.UpdateAttributeL(iServiceRecord, KSdpAttrIdServiceAvailability, state);
	// mark record changed
//	iSdpDB.UpdateAttributeL(iServiceRecord, KSdpAttrIdServiceRecordState,
//	    ++iRecordState);
}
