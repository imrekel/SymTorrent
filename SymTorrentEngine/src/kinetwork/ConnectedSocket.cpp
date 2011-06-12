/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of KiNetwork
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

#include "ConnectedSocket.h"
#include "ReadBuffer.h"
#include "ConnectedSocketReader.h"
#include "ConnectedSocketConnector.h"
#include <EscapeUtils.h>
#include <utf.h>

EXPORT_C CKiConnectedSocket::CKiConnectedSocket(/*TConnectedSocketType aType*/)
 //: iType(aType)
{
}

EXPORT_C void CKiConnectedSocket::ConstructL()
{
	ConstructL(NULL);
}

EXPORT_C void CKiConnectedSocket::ConstructL(RSocket* aSocket)
{
	if (aSocket)
		BaseConstructL(aSocket);
	else
		BaseConstructL();
	
	iRecvBuffer = new (ELeave) CReadBuffer;
	iRecvBuffer->ConstructL();

	iSocketReader = new (ELeave) CKiConnectedSocketReader(Socket(), *this, *iRecvBuffer);
	iSocketReader->ConstructL();

	iSocketWriter = new (ELeave) CKiConnectedSocketWriter(Socket(), *this);
	iSocketWriter->ConstructL();
	
	iSocketConnector = new (ELeave) CKiConnectedSocketConnector(*this);
	iSocketConnector->ConstructL();
}

EXPORT_C CKiConnectedSocket::~CKiConnectedSocket()
{
	delete iSocketReader;
	delete iSocketWriter;
	delete iSocketConnector;

	delete iRecvBuffer;
}

EXPORT_C void CKiConnectedSocket::OpenSocketL(TInt aConnIndex)
{
	iNetMgr->OpenConnectedSocketL(Socket(), this, aConnIndex);
	// little hack
	/*if (((MobileAgent::CMobileAgentDocument*)(CEikonEnv::Static()->EikAppUi()->Document()))->IsIAPSet())
		return iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp, ((MobileAgent::CMobileAgentDocument*)(CEikonEnv::Static()->EikAppUi()->Document()))->CommEngine()->NetworkConnection());
	else	
		return iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp);
		
	return ((MobileAgent::CMobileAgentDocument*)(CEikonEnv::Static()->EikAppUi()->Document()))->CommEngine()->OpenSocket(iSocket);*/
	
//	return NETWORKMGR->OpenSocket(iSocket);
	//return iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp);
}

EXPORT_C void CKiConnectedSocket::CloseSocket()
{
	iSocketReader->Cancel();
	iSocketWriter->Cancel();
	iNetMgr->Close(Socket());
}

EXPORT_C void CKiConnectedSocket::ConnectL(const TSockAddr& aAddress)
{
	iSocketConnector->ConnectL(aAddress);
}

EXPORT_C void CKiConnectedSocket::StopConnecting()
{
	iSocketConnector->Cancel();
}


EXPORT_C void CKiConnectedSocket::SendUrlEncodedL(const TDesC16& aDes)
{
	HBufC8* utf = HBufC8::NewLC(aDes.Size());
	TPtr8 ptr (utf->Des());
	CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, aDes);
	SendUrlEncodedL(*utf);	
	CleanupStack::PopAndDestroy(); // utf
}


EXPORT_C void CKiConnectedSocket::SendUrlEncodedL(const TDesC8& aDes) 
{
	HBufC8* encoded = EscapeUtils::EscapeEncodeL(aDes, EscapeUtils::EEscapeUrlEncoded);
	CleanupStack::PushL(encoded);
	iSocketWriter->WriteL(*encoded);
	CleanupStack::PopAndDestroy(); // encoded
}

EXPORT_C void CKiConnectedSocket::StartReceiving()
{
	iSocketReader->Start();
}

EXPORT_C void CKiConnectedSocket::StopReceiving()
{
	iSocketReader->Cancel();
}

EXPORT_C void CKiConnectedSocket::StopSending()
{
	iSocketWriter->Cancel();
}

EXPORT_C void CKiConnectedSocket::StopAll()
{
	StopConnecting();
	StopSending();
	StopReceiving();
}

