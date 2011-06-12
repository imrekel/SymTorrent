#include "UDPSocketWriter.h"
#include "UDPSocket.h"
#include "KiNetworkLog.h"

CKiDatagram* CKiDatagram::NewLC(TInetAddr& aAddress, const TDesC8& aPayLoad)
{
	CKiDatagram* instance = new (ELeave) CKiDatagram(aAddress);
	CleanupStack::PushL(instance);
	instance->ConstructL(aPayLoad);
	
	return instance;
}

CKiDatagram* CKiDatagram::NewL(TInetAddr& aAddress, const TDesC8& aPayLoad)
{
	CKiDatagram* instance = CKiDatagram::NewLC(aAddress, aPayLoad);
	CleanupStack::Pop();
	
	return instance;
}

void CKiDatagram::ConstructL(const TDesC8& aPayLoad)
{
	iPayLoad = aPayLoad.AllocL();
}


CKiUDPSocketWriter::CKiUDPSocketWriter(CKiUDPSocket& aSocket, CKiLogger* aLog)
 : CActive(EPriorityStandard), iSocket(aSocket)
{
	#ifdef LOG_TO_FILE
	iLog = aLog;
	#endif
}

void CKiUDPSocketWriter::ConstructL()
{
	CActiveScheduler::Add(this);
}

CKiUDPSocketWriter::~CKiUDPSocketWriter()
{
	Cancel();
	iDatagrams.ResetAndDestroy();
}

EXPORT_C void CKiUDPSocketWriter::SendDatagramL(TInetAddr& aRemoteAddress, const TDesC8& aDatagram)
{
	CKiDatagram* datagram = CKiDatagram::NewLC(aRemoteAddress, aDatagram);
	User::LeaveIfError(iDatagrams.Append(datagram));
	CleanupStack::Pop(datagram);
	
	if (!IsActive())
		IssueWrite();
}

void CKiUDPSocketWriter::DoCancel()
{
    // Cancel asychronous write request
	iSocket.Socket().CancelSend();
}

void CKiUDPSocketWriter::RunL()
{
    // Active object request complete handler
    switch (iStatus.Int())
    {
        case KErrNone:
            // write completed succesfully, delegating event to
			// SocketBase
		//	iSocketBase.OnSendCompleteL();
			
		//	LOG->WriteL(_L8("[Socket] out ::::"));
		//	LOG->WriteLineL(iShortBuffer);
        	
			#ifdef LOG_TO_FILE
			{
				TBuf<128> addrB;
				HLWRITE(iLog, _L("[CKiUDPSocketWriter] Datagram sent successfully to: "));
				iDatagrams[0]->Address().Output(addrB);
				addrB.Append(_L(":"));
				addrB.AppendNum(iDatagrams[0]->Address().Port());
				HLWRITE(iLog, addrB);
				HLWRITE(iLog,_L(" size: "));
				HLWRITELN(iLog, iDatagrams[0]->PayLoad().Length());
			}
			#endif
		
			// delete the sent datagram
			delete iDatagrams[0];
			iDatagrams.Remove(0);
			            
			if (iDatagrams.Count() > 0) 
				IssueWrite();
			
            break;
        default:
        	
			#ifdef LOG_TO_FILE
			{
				HLWRITE(iLog, _L("[CKiUDPSocketWriter] Failed to send datagram to: "));
				TBuf<128> addrBuf2;
				iDatagrams[0]->Address().Output(addrBuf2);
				addrBuf2.Append(_L(":"));
				addrBuf2.AppendNum(iDatagrams[0]->Address().Port());
				HLWRITE(iLog, addrBuf2);
				HLWRITE(iLog,_L(" size: "));
				HLWRITELN(iLog, iDatagrams[0]->PayLoad().Length());
			}
			#endif
        	
			iSocket.HandleWriteErrorL();
            break;
    }
}

void CKiUDPSocketWriter::IssueWrite()
{
	if (!IsActive() && (iDatagrams.Count() > 0))
	{		
		CKiDatagram* dg = iDatagrams[0];
		iSocket.Socket().SendTo(dg->PayLoad(), dg->Address(), 0, iStatus);
		iSocket.IncOutgoingTraffic(dg->PayLoad().Length());
		
		iSocket.OnDataSentL(dg->PayLoad().Length());
		SetActive();
	}   
}
