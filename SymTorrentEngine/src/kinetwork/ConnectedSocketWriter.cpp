/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrent
 *
 * SymTorrent is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SymTorrent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#include "ConnectedSocketWriter.h"
#include "ConnectedSocket.h"
#include "WriteBuffer.h"
#include "NetworkManager.h"
#include "KiNetworkLog.h"

CKiConnectedSocketWriter::CKiConnectedSocketWriter(RSocket& aSocket, CKiSocketBase& aSocketBase, TInt aShortBufferSize)
: CActive(EPriorityStandard),
  iSocket(aSocket), 
  iSocketBase(aSocketBase),
  iShortBufferSize(aShortBufferSize)
{
}


CKiConnectedSocketWriter::~CKiConnectedSocketWriter()
{
	Cancel();
	iObservers.Reset();
	iShortBuffer.Close();
	delete iLongBuffer;
}


void CKiConnectedSocketWriter::ConstructL()
{
	CActiveScheduler::Add(this);
	iShortBuffer.CreateL(iShortBufferSize);

	iLongBuffer = CWriteBuffer::NewL();	
}


void CKiConnectedSocketWriter::DoCancel()
{
    // Cancel asychronous write request
	iSocket.CancelWrite();
}


void CKiConnectedSocketWriter::RunL()
{
    // Active object request complete handler
    switch (iStatus.Int())
    {
        case KErrNone:
            // write completed succesfully, delegating event to
			// SocketBase
		//	iSocketBase.OnSendCompleteL();
        	//CKiLogger* log = NETMGR->LogL();
			//log->WriteL(_L8("[Socket] out ::::"));
			//log->WriteLineL(iShortBuffer);
        	
        	// notifying observers
        	if (iObservers.Count() > 0)
    		{
    			for (TInt i=0; i<iObservers.Count(); i++)
    				if (iObservers[i].iBufferIndex <= 0)
					{
						iObservers[i].iObserver->OnSocketWriteFinishedL();
						iObservers.Remove(i);
						i--;
					}
    		}
			            
			if (iLongBuffer->Size() > 0) IssueWrite();
            break;
        default:
			iSocketBase.HandleWriteErrorL();
            break;
    }
}


EXPORT_C void CKiConnectedSocketWriter::WriteL(const TDesC8& aBuf, MKiConnectedSocketWriterObserver* aObserver)
{
	iLongBuffer->AppendL(aBuf);
	
	if (aObserver)
		iObservers.AppendL(TKiSocketWriterObserverEntry(aObserver, iLongBuffer->Size()));

	if (!IsActive()) IssueWrite();
}


EXPORT_C void CKiConnectedSocketWriter::WriteWithoutSendingL(const TDesC8& aBuf, MKiConnectedSocketWriterObserver* aObserver)
{
	iLongBuffer->AppendL(aBuf);
	
	if (aObserver)
		iObservers.AppendL(TKiSocketWriterObserverEntry(aObserver, iLongBuffer->Size()));
}


EXPORT_C void CKiConnectedSocketWriter::SendNow()
{
	if (!IsActive()) IssueWrite();
}


void CKiConnectedSocketWriter::IssueWrite()
{
   // __ASSERT_ALWAYS(!IsActive(), User::Panic(KPaniCKiTCPSocketWriter, EActiveObjectIsActive));
	iLongBuffer->Read(iShortBuffer);
	
	if (iObservers.Count() > 0)
	{
		for (TInt i=0; i<iObservers.Count(); i++)
			iObservers[i].iBufferIndex -= iShortBuffer.Size();
	}
	
    iSocket.Write(iShortBuffer, iStatus);    
  //  LOG->WriteL(_L("send::::"));
  //  LOG->WriteLineL(iShortBuffer);
    SetActive();
    
    iSocketBase.OnDataSentL(iShortBuffer.Size());
}
