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

#include "UDPSocketReader.h"
#include "UDPSocket.h"
#include "KiNetworkLog.h"

CKiUDPSocketReader::CKiUDPSocketReader(CKiUDPSocket& aSocket, CKiLogger* aLog)
 : CActive(EPriorityStandard), iSocket(aSocket), iStopped(ETrue)
{
	#ifdef LOG_TO_FILE
	iLog = aLog;
	#endif
}


CKiUDPSocketReader::~CKiUDPSocketReader()
{
	Cancel();
}


void CKiUDPSocketReader::ConstructL()
{
	CActiveScheduler::Add(this);
}


void CKiUDPSocketReader::DoCancel()
{
    // Cancel asychronous receive request
	iSocket.Socket().CancelRecv();
}


void CKiUDPSocketReader::RunL()
{
    // Active object request complete handler
    switch (iStatus.Int())
    {
        case KErrNone:		
        {
        	HLWRITELN(iLog, _L("[CKiUDPSocketReader] UDP datagram received"));
			//iLog->WriteL(_L8("[Socket] In: "));
			//iLog->WriteLineL(iBuffer);
        
        	iSocket.IncIncomingTraffic(iBuffer.Length());
			
            iSocket.OnReceiveL(iRemoteAddress, iBuffer);
        }
        break;
			
        default:
        {
        	HLWRITE(iLog, _L("[CKiUDPSocketReader] UDP read error"));
        	HLWRITELN(iLog, iStatus.Int());
			//iLog->WriteL(_L8("[Socket] Recv error: "));
			//iLog->WriteLineL(iStatus.Int());

			// closing connection
			iSocket.HandleReadErrorL();
		}
        break;
    }
    if (!iStopped)
    	IssueRead(); // Immediately start another read
}

void CKiUDPSocketReader::IssueRead()
{
    // Initiate a new read from socket into iBuffer
//    __ASSERT_ALWAYS(!IsActive(), User::Panic(KPanicSocketReader, EActiveObjectIsActive));
	if (!IsActive())
	{
	//	iLog->WriteLineL(_L8("Issue recv"));
		iBuffer.SetLength(0);
		iSocket.Socket().RecvFrom(iBuffer, iRemoteAddress, 0, iStatus);
    	SetActive();
	}  
}

void CKiUDPSocketReader::StartL(TUint aPort)
{
	HLWRITELN(iLog, _L("[CKiUDPSocketReader] Starting"));
    // Initiate a new read from socket into iBuffer
    if (!IsActive())
    {
	    TInetAddr addr(KInetAddrAny, aPort);
		User::LeaveIfError( iSocket.Socket().Bind(addr) );
		
		iStopped = EFalse;
		
        IssueRead();
    }
}

void CKiUDPSocketReader::Start()
{
	if (!IsActive())
	{
		iStopped = EFalse;
		IssueRead();
	}
		
}

void CKiUDPSocketReader::Stop()
{
	iStopped = ETrue;
	Cancel();
	LWRITELN(iLog, _L8("UDP receiving STOPPED"));
}
