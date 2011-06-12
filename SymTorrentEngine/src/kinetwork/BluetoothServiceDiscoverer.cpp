/*
 ============================================================================
 Name		: BluetoothServiceDiscoverer.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CBluetoothServiceDiscoverer implementation
 ============================================================================
 */

#include "BluetoothServiceDiscoverer.h"
#include "BluetoothCommon.h"

//#include "NetworkManager.h"
//#include "KiLogger.h"
//#define LOG NETMGR->LogL()

CBluetoothServiceDiscoverer::CBluetoothServiceDiscoverer()
{
	
}

CBluetoothServiceDiscoverer::~CBluetoothServiceDiscoverer()
{
	iDevicesToDiscover.Reset();
	delete iSdpAgent;
}

CBluetoothServiceDiscoverer* CBluetoothServiceDiscoverer::NewLC()
{
	CBluetoothServiceDiscoverer* self = new (ELeave)CBluetoothServiceDiscoverer();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CBluetoothServiceDiscoverer* CBluetoothServiceDiscoverer::NewL()
{
	CBluetoothServiceDiscoverer* self=CBluetoothServiceDiscoverer::NewLC();
	CleanupStack::Pop(); // self;
	return self;
}

void CBluetoothServiceDiscoverer::ConstructL()
{

}

void CBluetoothServiceDiscoverer::DiscoverServiceL(const TBTDevAddr& aAddr)
{
	iDevicesToDiscover.Append(aAddr);
	
	if (iDevicesToDiscover.Count() == 1)
		StartDiscoverL();
}

void CBluetoothServiceDiscoverer::StartDiscoverL()
{
	delete iSdpAgent;
	iSdpAgent = NULL;
	iGotService = EFalse;
	
	iSdpAgent = CSdpAgent::NewL(*this, iDevicesToDiscover[0]);

	CSdpSearchPattern *pattern=
		CSdpSearchPattern::NewL();
	CleanupStack::PushL(pattern);	
	pattern->AddL(KSdpServiceClassSerialPort);
	iSdpAgent->SetRecordFilterL(*pattern);
	CleanupStack::PopAndDestroy(); // pattern

	iSdpAgent->NextRecordRequestL();
}

void CBluetoothServiceDiscoverer::NextRecordRequestComplete(TInt aError,
	TSdpServRecordHandle aHandle, TInt /*aTotalRecordsCount*/)
{
	if(aError != KErrNone)
	{
		if (!iGotService && iObserver)
			iObserver->ReportBtServiceDiscoveryErrorL(iDevicesToDiscover[0], aError);
		
		iDevicesToDiscover.Remove(0);
		if (iDevicesToDiscover.Count() > 0)
			StartDiscoverL();
		else
		{
			delete iSdpAgent;
			iSdpAgent = NULL;
		}
	}
    else
    {
        iSdpAgent->AttributeRequestL(aHandle, KSdpAttrIdBasePrimaryLanguage
			+ KSdpAttrIdOffsetServiceDescription);
    }
}

void CBluetoothServiceDiscoverer::AttributeRequestResult(TSdpServRecordHandle /*aHandle*/,
	TSdpAttributeID /*aAttrID*/, CSdpAttrValue* aAttrValue)
{
	if(aAttrValue->Type() == ETypeString)
    {
        TBuf<128> buf;
        buf.Copy(aAttrValue->Des());
        
   //     delete aAttrValue;
        
        _LIT(KPattern,"Channel: *");
        if(buf.Match(KPattern) == 0)
        {
        	iGotService = ETrue;
        	
            TLex lex(buf);
            lex.SkipCharacters();
            lex.SkipSpace();
            
            TInt channel;
            TInt res = lex.Val(channel);
            if (res == KErrNone)
        	{
        		if (iObserver)
        			iObserver->OnBtServiceDiscoveryCompleteL(iDevicesToDiscover[0], channel);
        	}
            else
        	{
        		if (iObserver)
        			iObserver->ReportBtServiceDiscoveryErrorL(iDevicesToDiscover[0], res);
        	}
        }
    }
//	else
//		delete aAttrValue;
}

void CBluetoothServiceDiscoverer::AttributeRequestComplete(TSdpServRecordHandle /*aHandle*/, TInt /*aError*/)
{	
	iSdpAgent->NextRecordRequestL();
}

