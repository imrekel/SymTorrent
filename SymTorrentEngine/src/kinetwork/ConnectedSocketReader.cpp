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

#include "ConnectedSocketReader.h"
#include "ConnectedSocket.h"
#include "ReadBuffer.h"
#include "NetworkManager.h"
#include "KiLogManager.h"

CKiConnectedSocketReader::CKiConnectedSocketReader(RSocket& aSocket, 
								 CKiConnectedSocket& aSocketBase,
								 CReadBuffer& aLongBuffer)
: CActive(EPriorityStandard),
  iSocket(aSocket), 
  iSocketBase(aSocketBase),
  iLongBuffer(aLongBuffer)
{
}


CKiConnectedSocketReader::~CKiConnectedSocketReader()
{
	Cancel();
}


void CKiConnectedSocketReader::ConstructL()
{
	CActiveScheduler::Add(this);

	#ifdef LOG_TO_FILE
	iLog = NETMGR->LogL();
	#endif
}


void CKiConnectedSocketReader::DoCancel()
{
    // Cancel asychronous receive request
	iSocket.CancelRecv();
}


void CKiConnectedSocketReader::RunL()
{
    // Active object request complete handler
    switch (iStatus.Int())
    {
        case KErrNone:
            // Character has been read from socket			
			iLongBuffer.AppendL(iBuffer);
			
			//iLog->WriteL(_L8("[Socket] in  ::::"));
			//iLog->WriteLineL(iBuffer);

			
			iBuffer.SetLength(0);			
		    IssueRead(); // Immediately start another read		
			
            iSocketBase.OnReceiveL();
			
            break;
			
        default:
			LWRITE(iLog, _L8("(reader) socket error "));
			LWRITELN(iLog, iStatus.Int());

			// closing connection
			iSocketBase.HandleReadErrorL();
            break;
    }	
}

void CKiConnectedSocketReader::IssueRead()
{
    // Initiate a new read from socket into iBuffer
//    __ASSERT_ALWAYS(!IsActive(), User::Panic(KPaniCKiTCPSocketReader, EActiveObjectIsActive));
	if (!IsActive())
	{
		iSocket.RecvOneOrMore(iBuffer, 0, iStatus, iLastRecvLength);
    	SetActive();
	}  
}

void CKiConnectedSocketReader::Start()
{
    // Initiate a new read from socket into iBuffer
    if (!IsActive())
    {
        IssueRead();
    }
}
