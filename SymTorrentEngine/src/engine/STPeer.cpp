/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
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

/* ============================================================================
 *  Name     : CSTPeer from STPeer.cpp
 *  Part of  : SymTorrent
 *  Created  : 20.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STPeer.h"
#include "STBitField.h"
#include "STTorrent.h"
#include "SymTorrentEngineLog.h"
#include <in_sock.h>
#include "SymTorrentEngineLog.h"
#include "STDefs.h"
#include "FailedPieceCollector.h"

const TInt KMaxPeerErrorCount = 6;

const TInt KNormalModeReconnectDelay = 60;
const TInt KLocalPeerReconnectDelay = 3;

const TInt KEndGameModeReconnectDelay = 30;

// ================= MEMBER FUNCTIONS =======================


CSTPeer* CSTPeer::NewL(const TSockAddr& aAddress, const TDesC8& aPeerId, TInt aBitFieldLength, TBool aLocal, TBool aPushPeer)
{
	CSTPeer* self = CSTPeer::NewLC(aAddress, aPeerId, aBitFieldLength, aLocal, aPushPeer);
	CleanupStack::Pop();
	
	return self;
}
	
CSTPeer* CSTPeer::NewLC(const TSockAddr& aAddress, const TDesC8& aPeerId, TInt aBitFieldLength, TBool aLocal, TBool aPushPeer)
{
	CSTPeer* self = new (ELeave) CSTPeer(aAddress, aLocal, aPushPeer);
	CleanupStack::PushL(self);
	self->ConstructL(aPeerId, aBitFieldLength);
	
	return self;
}

CSTPeer* CSTPeer::NewL(const TSockAddr& aAddress,  TInt aBitFieldLength, TBool aLocal, TBool aPushPeer)
{
	CSTPeer* self = CSTPeer::NewLC(aAddress, aBitFieldLength, aLocal, aPushPeer);
	CleanupStack::Pop();
	
	return self;
}

CSTPeer* CSTPeer::NewLC(const TSockAddr& aAddress, TInt aBitFieldLength, TBool aLocal, TBool aPushPeer)
{
	CSTPeer* self = new (ELeave) CSTPeer(aAddress, aLocal, aPushPeer);
	CleanupStack::PushL(self);
	self->ConstructL(aBitFieldLength);
	
	return self;
}

CSTPeer::CSTPeer(const TSockAddr& aAddress, TBool aLocal, TBool aPushPeer)
 : iAddress(aAddress), iPushPeer(aPushPeer), iLocal(aLocal)
{
}

CSTPeer* CSTPeer::NewL(RSocket* aSocket, CSTTorrentManager* aTorrentMgr, TBool aLocal)
{
	CSTPeer* self = CSTPeer::NewLC(aSocket, aTorrentMgr, aLocal);
	CleanupStack::Pop();
	
	return self;
}
	
CSTPeer* CSTPeer::NewLC(RSocket* aSocket, CSTTorrentManager* aTorrentMgr, TBool aLocal)
{
	TSockAddr addr;
	CSTPeer* self = new (ELeave) CSTPeer(addr, aLocal);
	CleanupStack::PushL(self);
	self->ConstructL(aSocket, aTorrentMgr);
	
	return self;
}

void CSTPeer::ConstructL(RSocket* aSocket, CSTTorrentManager* aTorrentMgr)
{
	ConstructL(1);
	
	(*aSocket).RemoteName(iAddress);
	
	iConnection = new (ELeave) CSTPeerConnection(*this, NULL, aTorrentMgr);
	iConnection->ConstructL(aSocket);
}

void CSTPeer::ConstructL(const TDesC8& aPeerId, TInt aBitFieldLength)
{	
	if (aPeerId.Length() != 20)
		User::Leave(KErrGeneral);
	
	iPeerId = aPeerId.AllocL();	
	
	ConstructL(aBitFieldLength);
}

void CSTPeer::ConstructL(TInt aBitFieldLength)
{	
	iBitField = new (ELeave) CSTBitField;
	iBitField->ConstructL(aBitFieldLength);
	iFailedPieceCollector = CFailedPieceCollector::NewL();
}

CSTPeer::~CSTPeer()
{
	delete iConnection;
	delete iBitField;
	delete iPeerId;
	delete iFailedPieceCollector;
}


void CSTPeer::ConnectL(CSTTorrent& aTorrent, CSTTorrentManager* aTorrentMgr)
{
	SetReconnectTime(0);
	
	if  (!iConnection)	
	{
		iConnection = new (ELeave) CSTPeerConnection(*this, &aTorrent, aTorrentMgr);
		iConnection->ConstructL();		
	}
	
	if (iConnection->State() == EPeerNotConnected)
		iConnection->ConnectL();
}


void CSTPeer::OnTimerL(CSTTorrent* aTorrent, TInt aTorrentEllapsedTime)
{
	if (iConnection)
	{
		iConnection->OnTimerL();
		
		if (iConnection->State() == EPeerClosing)
		{
			if (IsLocal())
				SetReconnectTime(aTorrentEllapsedTime + KLocalPeerReconnectDelay);
			else
			{
				switch (iConnection->CloseOrder())
				{
					case CSTPeerConnection::EIncreaseErrorCounter:
						iErrorCounter++;
					break;
					
					case CSTPeerConnection::EDeletePeer:
						iDeletable = ETrue;
					break;
					
					case CSTPeerConnection::EDelayReconnect:
						if (aTorrent)
						{												
							if (aTorrent->EndGame())
								SetReconnectTime(aTorrentEllapsedTime + KEndGameModeReconnectDelay);
							else
								SetReconnectTime(aTorrentEllapsedTime + KNormalModeReconnectDelay);	
						}									
					break;
					
					default:
					break;
				}
			}
			
			if (iErrorCounter >= KMaxPeerErrorCount)
			{				
				LWRITELN(STLOG, _L("[Peer] Max error count reached, deleting peer."));
				iDeletable = ETrue;
			}
			
			delete iConnection;
			iConnection = NULL;					
		}		
	}
}

void CSTPeer::HavePiece(TInt aPieceIndex, CSTTorrent& aTorrent)
{
	HLWRITELN(STLOG, _L("[Peer] HavePiece begin"));
	
	if (!iBitField->IsBitSet(aPieceIndex))
		aTorrent.IncreaseNumberOfPeersHavePiece(aPieceIndex);
	
	iBitField->SetBit(aPieceIndex);
	
	HLWRITELN(STLOG, _L("[Peer] HavePiece end"));
}
	
void CSTPeer::HavePiecesL(const TDesC8& aBitFieldDes, CSTTorrent& aTorrent)
{
	CSTBitField* bitfield = new (ELeave) CSTBitField;
	CleanupStack::PushL(bitfield);
	bitfield->ConstructL(aTorrent.PieceCount());
	bitfield->SetL(aBitFieldDes);
	
	for (TInt i=0; i<aTorrent.PieceCount(); i++)
	{
		if (bitfield->IsBitSet(i) && (!iBitField->IsBitSet(i)))
			aTorrent.IncreaseNumberOfPeersHavePiece(i);
	}
	
	CleanupStack::PopAndDestroy(); // bitfield
	
	iBitField->SetL(aBitFieldDes);
}

void CSTPeer::NotifyThatClientHavePiece(TInt aIndex)
{
	HLWRITELN(STLOG, _L("[Peer] NotifyThatClientHavePiece begin"));
	
	if (iConnection)
	{
		if (State() == EPeerPwConnected)
			if (!BitField()->IsBitSet(aIndex))
				iConnection->SendHaveMessageL(aIndex);
	}
	
	HLWRITELN(STLOG, _L("[Peer] NotifyThatClientHavePiece end"));
}

void CSTPeer::SetBitFieldLengthL(TInt aLength)
{
	delete iBitField;
	iBitField = 0;
	
	iBitField = new (ELeave) CSTBitField;
	iBitField->ConstructL(aLength);
}

void CSTPeer::DisconnectL()
{
	if (iConnection)
	{
		iConnection->CloseL(_L8("Torrent paused"));
	}
}

void CSTPeer::ResetAddress()
{
	if (iConnection)
	{
		iConnection->RemoteAddress(iAddress);
	}		
	else
	{
		TSockAddr addr;
		iAddress = addr;
		//iAddress.SetAddress(0);
		//iAddress.SetPort(0);
	}
}

void CSTPeer::CancelPieceRequestL(CSTPiece* aPiece)
{
	if (iConnection)
		iConnection->CancelPieceRequestL(aPiece);
}

void CSTPeer::SetPeerIdL(const TDesC8& aPeerId)
{
	delete iPeerId;
	iPeerId = NULL;
	iPeerId = aPeerId.AllocL();
}

void CSTPeer::AttachConnectionL(CSTPeerConnection* aConn)
{
	HLWRITELN(STLOG, _L("[Peer] AttachConnection begin"));
	
	if (iConnection)
	{
		if (iConnection->State() != EPeerClosing)
		{
			iConnection->CloseL(_L8("New connection attached to the peer"));
		}
		
		delete iConnection;
	}
	
	iConnection = aConn;
	iConnection->SetPeer(this);
	
	HLWRITELN(STLOG, _L("[Peer] AttachConnection end"));
}

void CSTPeer::DetachConnection()
{
	HLWRITELN(STLOG, _L("[Peer] DetachConnection begin"));
	
	iConnection = NULL;
	
	HLWRITELN(STLOG, _L("[Peer] DetachConnection end"));
}

void CSTPeer::DeleteL()
{
	if (iConnection)
		iConnection->CloseL(CSTPeerConnection::EDeletePeer, _L8("Peer is already added with local connection!"));
	else
		iDeletable = ETrue;
}

TPeerConnectionState CSTPeer::State() const 
{
	if (iConnection) 
		return iConnection->State();
	
	return EPeerNotConnected;
}

