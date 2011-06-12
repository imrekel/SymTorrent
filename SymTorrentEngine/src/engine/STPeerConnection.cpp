/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrentEngine
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
 *  Name     : CSTPeerConnection from STPeerConnection.cpp
 *  Part of  : SymTorrent
 *  Created  : 22.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STPeerConnection.h"
#include "STPeer.h"
#include "STTorrent.h"
#include "STTorrentManager.h"
#include "STPiece.h"
#include "WriteBuffer.h"
#include "ReadBuffer.h"
//#include "SocketReader.h"
#include "SymTorrentEngineLog.h"
#include "STBitField.h"
#include "NetworkManager.h"
#include "STPreferences.h"
#include "ST.pan"
#include "STUtils.h"
#include "NetworkManager.h"
//#include "STPieceAccess.h"
#include <utf.h>
#include <badesca.h>
#include "SymTorrentEngineLog.h"

// CONSTS
_LIT8(KLitProtocolId, "BitTorrent protocol");
const TInt KTcpConnectTimeout = 25;
const TInt KHandshakeTimeout = 20;
const TInt KPwConnectionTimeout = 4*60;
const TInt KKeepAliveInterval = 2*60; // once in every two minutes

const TInt KRequestTimeout = 30;
const TInt KLocalRequestTimeoutUDP = 15;

const TInt KDefaultBlockSize = 16384; // default download block size (2^14)
const TInt KDefaultBlockSizeLocalConnectionUDP = 1024;
const TInt KMinimumBlockSize = 16384; // (2^14)
const TInt KMaximumBlockSize = 131072; // (2^17)

const TInt KMaxPieceRequests = 3;
const TInt KMaxPieceRequestsLocalConnection = 5;

const TUint8 KGridTorrentExtensionByte = 16; // 4th byte of the reserverd section

const TInt KMaxPieceRequestQueueSize = 10;
const TInt KMaxPieceRequestQueueSizeLocalConnection = 10;

// ================= CSTPieceToDownload MEMBER FUNCTIONS =======================

CSTPieceToDownload::CSTPieceToDownload(CSTPiece* aPiece, TInt aLastRequestTime)
	 : iPiece(aPiece), iLastRequestTime(aLastRequestTime) //, iLastRequestBegin(-1), iLastRequestLength(-1)
{
	iRequestedSize = aPiece->DownloadedSize();
}

// ================= CSTPeerConnection MEMBER FUNCTIONS =======================

CSTPeerConnection::CSTPeerConnection(CSTPeer& aPeer, CSTTorrent* aTorrent, CSTTorrentManager* aTorrentMgr)
 : iPeer(&aPeer), 
   iTorrent(aTorrent), 
   iIncomingConnection(EFalse), 
   iTorrentMgr(aTorrentMgr), 
   iGridTorrentExtensionEnabled(EFalse)
{
}

void CSTPeerConnection::ConstructL()
{
	#ifdef LOG_TO_FILE
	iLog = STLOG;
	#endif
	
	CKiConnectedSocket::ConstructL();
	
	SetInterestedL(EFalse);
	SetPeerInterestedL(EFalse);
	
	SetChokingL(ETrue);
	SetPeerChoking(ETrue);	
}


void CSTPeerConnection::ConstructL(RSocket* aSocket)
{
	#ifdef LOG_TO_FILE
	iLog = STLOG;
	#endif
	
	iIncomingConnection = ETrue;
	iRetries++;
	
	/*TName socketName;
	TInt res;
	
	if ((res = aSocket.Name(socketName)) == KErrNone)
	{
		iSocket.Close();
		
		res = iSocket.Transfer(iNetMgr->SocketServ(), socketName);
	}
	
	if (res != KErrNone)
	{
		CloseL(EDeletePeer, _L8("Failed to transfer socket!"));
		return;
	}*/
	
/*	iSocket.Close();								
	iSocket.~RSocket();
	
	Mem::Copy(&iSocket, &aSocket, sizeof(aSocket));	// HACK!!!!!!!!!!!!!!!!! (socket transfer doesn't work)*/
	
	CKiConnectedSocket::ConstructL(aSocket);
//	Socket().SetOpt(KSoTcpNoDelay, KSolInetTcp, 1);
	
	SetInterestedL(EFalse);
	SetPeerInterestedL(EFalse);
	
	SetChokingL(ETrue);
	SetPeerChoking(ETrue);
	
	TorrentMgr()->IncIncomingConnectionCountL();
	
	ChangeState(EPeerPwHandshaking);
	StartReceiving();			
	//SendHandshakeMessageL();
	
	LogL(_L("ConstructL(Rsocket*) end"));
}


CSTPeerConnection::~CSTPeerConnection()
{
	//iTorrent->PieceAccess()->CancelAllWrite(iPeer);
	iIncomingRequests.Reset();
	iPiecesToDownload.ResetAndDestroy();
}


void CSTPeerConnection::ConnectL()
{
	iRetries++;
//	iReconnectAfter = 0;
    /*__ASSERT_DEBUG( (iState == ENotConnected) || (iState == EResolving), 
		User::Panic(KSAPanicNode, ESAAlreadyConnected));
	
	__ASSERT_ALWAYS(!IsActive(), 
		User::Panic(KSAPanicNode, ESAActiveObjectIsActive));  */

	CloseSocket();
	
	LogL(_L("Connecting"));
	ChangeState(EPeerTcpConnecting);
	
	// SocketOpenedL called by the framework
	if (iPeer->IsLocal())
		OpenSocketL(1); 
	else
		OpenSocketL(0);
}


void CSTPeerConnection::SocketOpenedL(TBool aResult, RSocket& /*aSocket*/)
{
	if (aResult)
		CKiConnectedSocket::ConnectL(RemoteAddress());
	else
	{
		CloseL(EIncreaseErrorCounter, _L8("Failed to open socket..."));
		
		/*if (iTorrent)
			iTorrent->SetFailedL(_L("Failed to establish network connection."));*/
	}
}


void CSTPeerConnection::LogL(const TDesC& aText)
{
#ifdef LOG_TO_FILE
	TBuf<100> address;
	
	if (iPeer->IsLocal())
	{		
		iLog->WriteL(_L("[LocalPeer "));
		if (iTorrentMgr->NetworkManager()->Connection(1).Type() == CNetworkConnection::ERConnectionBased)
		{
			TInetAddr addr(iPeer->Address());
			addr.Output(address);
		}
		else
		{
			iLog->WriteL((TInt)&iPeer);
			iLog->WriteL(_L(" "));
		}
	}
	else
	{
		TInetAddr addr(iPeer->Address());
		addr.Output(address);
		iLog->WriteL(_L("[Peer "));
	}
	
	if (address.Length() > 0)
		iLog->WriteL(address);
	iLog->WriteL(_L(":"));
	address.Num(iPeer->Address().Port());
	iLog->WriteL(address);
	
	if (iIncomingConnection)
		iLog->WriteL(_L(" in] "));
	else
		iLog->WriteL(_L(" out] "));
	
	/*TTime now;
	now.HomeTime();
	TDateTime dateTime = now.DateTime();
	TBuf<30> timeBuf;
	timeBuf.Append(_L("]["));
	timeBuf.AppendNum(dateTime.Hour());
	timeBuf.Append(TChar(':'));
	timeBuf.AppendNum(dateTime.Minute());
	timeBuf.Append(TChar(':'));
	timeBuf.AppendNum(dateTime.Second());
	timeBuf.Append(_L("] "));
	iLog->WriteL(timeBuf);*/
	
	iLog->WriteLineL(aText);
#endif
}


void CSTPeerConnection::OnReceiveL()
{
	HLWRITELN(iLog, _L("[PeerConn] OnReceiveL begin"));
	/*if (iIncomingConnection)
		LogL(_L("OnReceiveL()"));*/
	
	switch (iState)
	{		
		case EPeerPwHandshaking:
		{
			if (iRecvBuffer->Size() > 0)
			{
				TInt protLength = (iRecvBuffer->Ptr())[0];				
				TInt handshakeLength = protLength + 1 + 48;
				
				if (iRecvBuffer->Size() >= handshakeLength)
				{
					LogL(_L("Parsing incoming handshake"));
					
					HLWRITELN(iLog, iRecvBuffer->Ptr().Left(handshakeLength));
					
					TLex8 lex(iRecvBuffer->Ptr());
					lex.Inc();
					lex.Mark();
					lex.Inc(protLength);
										
					if (lex.MarkedToken() != KLitProtocolId)
					{
						CloseL(EDeletePeer, _L8("Protocol identifier doesn't match!"));
						return;
					}
					
					lex.Inc(3);
					if (lex.Peek() && KGridTorrentExtensionByte)
					{
						iGridTorrentExtensionEnabled = ETrue;
						LogL(_L("Supports GridTorrent extension"));
					}											
					lex.Inc(5);	
					
					// parsing torrent infohash														
					lex.Mark();
					lex.Inc(20);
					TPtrC8 infohash = lex.MarkedToken();
					
					// parsing peerId
					lex.Mark();
					lex.Inc(20);
					TPtrC8 peerId = lex.MarkedToken();
					
					// checking infohash
					if (iTorrent)
					{
						if (infohash != iTorrent->InfoHash())
						{
							CloseL(EDeletePeer, _L8("Torrent infohash doesn't match!"));
							return;
						}
					}
					else
					{
						LogL(_L("Trying to attach to torrent"));
						if ((TorrentMgr()->AttachPeerToTorrentL(infohash, peerId, this) != KErrNone)
							|| (iTorrent == NULL))
						{
							CloseL(EDeletePeer, _L8("Invalid infohash or peer is already connected or too many peers!"));
							return;
						}
						LogL(_L("Attached to torrent successfully"));
					}
					
					if (iIncomingConnection)
					{
						iPeer->ResetAddress();
						SendHandshakeMessageL();
					}								
					
					if (peerId == TorrentMgr()->PeerId())
					{
						CloseL(EDeletePeer, _L8("Connected to ourselves!"));
						return;
					}										

					if (iPeer->PeerId() != KNullDesC8)
					{
						if (peerId != iPeer->PeerId())
						{
							//CloseL(EIncreaseErrorCounter, _L8("Peer ID doesn't match!"));
							//return;
							LogL(_L("Warning, peer ID does not match!"));
						}												
					}
					else
					{
						if (!iPeer->IsLocal())
						{
							CSTPeer* peer = iTorrent->GetPeer(peerId);
							if (peer && peer->IsLocal())
							{
								CloseL(EDeletePeer, _L8("Peer is already added with local connection!"));
								return;
							}							
						}
						else
						{
							iTorrent->DeletePrimaryPeerL(peerId);
						}
						
						iPeer->SetPeerIdL(peerId);
					}					
															
					iRecvBuffer->Delete(0, handshakeLength);
					
					if (iPeer->IsLocal())
						Torrent().iLocalBytesDownloaded += handshakeLength;
					else
						Torrent().iLongRangeBytesDownloaded += handshakeLength;
					
					LogL(_L("Handshake completed! Peer wire connected!"));
					ChangeState(EPeerPwConnected);	
					SetPeerWireConnectedL();
					iPeer->ResetErrorCounter();
					
					// sending Bitfield
					if (!iTorrent->BitField()->IsNull())
						SendBitfieldMessageL();
					
					// sending local peers if not in master slave mode
					if (iGridTorrentExtensionEnabled && (!iTorrentMgr->IsLocalConnMasterSlaveMode()))
					{
						RPointerArray<CSTPeer> localPeers;
						CleanupClosePushL(localPeers);
						iTorrent->GetLocalPeersL(localPeers);
						
						for (TInt i=0; i<localPeers.Count(); i++)
						{
							if ((localPeers[i]->State() == EPeerPwConnected) &&
								(localPeers[i] != iPeer))
							{
								// we assume that these are IP addresses
								TInetAddr addr(localPeers[i]->Address());
								SendLocalPeerMessageL(addr.Address(), addr.Port());
							}
						}
						
						CleanupStack::PopAndDestroy(); // localPeers
					}
					
				//	iSocketReader->Start();
					
					OnReceiveL(); // explicit call to handle incoming messages which may have come along with the handshake
				}								
			}			
		}
		break;
		
		case EPeerPwConnected:
		{
			while (iRecvBuffer->Size() >= 4)
			{
				TUint messageLength = ReadInt();
				if (TUint(iRecvBuffer->Size()) >= (4 + messageLength))
				{
					iLastMessageReceivedTime = iEllapsedTime;
					if (messageLength == 0)
					{
						LogL(_L("in KEEPALIVE"));
					}
					else
					{
						TPtrC8 recv = iRecvBuffer->Ptr();
						
						switch (recv[4])
						{
							case KMessageIdBitfield:
							{
								LogL(_L("in BITFIELD"));
								if (messageLength-1 != TUint(iPeer->BitField()->LengthInBytes()))
								{
									CloseL(EIncreaseErrorCounter, _L8("Received bitfield length doesn't match!"));									
								}
								else
								{
									iPeer->HavePiecesL(recv.Mid(5, messageLength-1), *iTorrent);
									
									if (iPeer->IsLocal())
										iTorrent->HaveLocalNetPiecesL(recv.Mid(5, messageLength-1), *iPeer);
									
									IssueDownloadL();
								}
							}
							break;
							
							case KMessageIdHave:
							{	
								TInt pieceIndex = ReadInt(5);
								
								#ifdef LOG_TO_FILE
									TBuf<128> log;
									log.Format(_L("in HAVE %d"), pieceIndex);																												
									LogL(log);
								#endif																
								
								if ((pieceIndex >= 0) && (pieceIndex < iTorrent->PieceCount()))
								{
									iPeer->HavePiece(pieceIndex, *iTorrent);
									
									if (iPeer->IsLocal())
										iTorrent->HaveLocalNetPieceL(pieceIndex, *iPeer);
									
									IssueDownloadL();									
								}
								else
									CloseL(_L8("Error, piece index is out of bounds"));
									
							}
							break;
							
							case KMessageIdChoke:
								iPeer->ResetErrorCounter();
								LogL(_L("in CHOKE"));
								SetPeerChoking(ETrue);
								for (TInt i=0; i< iPiecesToDownload.Count(); i++)
								{
									iPiecesToDownload[i]->iPendingRequests.Reset();
									iPiecesToDownload[i]->iRequestedSize =
										iPiecesToDownload[i]->iPiece->DownloadedSize();
								}
									
								/*if (iPieceToDownload)
								{
									iTorrent->RemovePieceFromDownloading(iPieceToDownload);
									iPieceToDownload = NULL;
								}								
								iHasPendingDownloadRequest = EFalse;*/
								
							/*	if (iPiecesToDownload.Count() > 0)
								{
									for (TInt i=0; i< iPiecesToDownload.Count(); i++)
										iTorrent->RemovePieceFromDownloading(iPiecesToDownload[i]);
										
									iPiecesToDownload.Reset();
								}*/
							break;
							
							case KMessageIdUnchoke:
								iPeer->ResetErrorCounter();
								LogL(_L("in UNCHOKE"));
								SetPeerChoking(EFalse);
								
							/*	for (TInt i=0; i<iPiecesToDownload.Count(); i++)
									SendRequestMessageL(iPiecesToDownload[i].iPiece);*/
									
								IssueDownloadL();
							break;
							
							case KMessageIdInterested:
								iPeer->ResetErrorCounter();
								LogL(_L("in INTERESTED"));
								
								if (Preferences()->IsUploadEnabled())
								{
									SetPeerInterestedL(ETrue);
									SetChokingL(EFalse);
								}
								
							break;
							
							case KMessageIdNotInterested:
								iPeer->ResetErrorCounter();
								LogL(_L("in NOTINTERESTED"));
								SetPeerInterestedL(EFalse);
								IssueDownloadL();
							break;
							
							case KMessageIdPiece:
							{
								iPeer->ResetErrorCounter();
								iLastRequestTime = 0;
								iPeer->SetHadRequestTimeout(EFalse);
								
								TInt index = ReadInt(5);
								TInt begin = ReadInt(9);

								#ifdef LOG_TO_FILE
									TBuf<128> status;
									status.Format(_L("in PIECE Index: %d Begin: %d Length: %d"), index, begin, messageLength - 9);
									LogL(status);
								#endif
								
								if (!iPeer->IsLocal())
								{
									Torrent().iLongRangeSubPiecesReceived++;
									Torrent().iLongRangeSubPiecesReceivedSize += (messageLength - 9);
								}
								
								HandlePieceMessageL(index, begin, recv.Mid(13, messageLength - 9));																								
							}
							break;
							
							case KMessageIdRequest:
							{
								iPeer->ResetErrorCounter();
								if (messageLength < 13)
								{
									CloseL(EIncreaseErrorCounter, _L8("Received REQUEST message length is smaller than 13!"));
								}
								else
								{
									
									TInt pieceIndex = ReadInt(5);
									TInt begin = ReadInt(9);
									TInt length = ReadInt(13);
									
									#ifdef LOG_TO_FILE
										TBuf<128> status;
										status.Format(_L("in REQUEST Index: %d Begin: %d Length: %d"), pieceIndex, begin, length);
										LogL(status);
									#endif
									
									if (Preferences()->IsUploadEnabled())
									{	
										iIncomingRequests.Append(TSTBlockRequest(pieceIndex, begin, length));
										
										IssueUploadL();
									}
								}								
							}
							break;							
								
							case KMessageIdCancel:
							{
								LogL(_L("in CANCEL"));
								
								iPeer->ResetErrorCounter();
								if (messageLength < 13)
								{
									CloseL(EIncreaseErrorCounter, _L8("Received CANCEL message length is smaller than 13!"));
								}
								else
								{
									TInt pieceIndex = ReadInt(5);
									TInt begin = ReadInt(9);
									TInt length = ReadInt(13);
									
									for (TInt i=0; i<iIncomingRequests.Count(); i++)
									{
										if ((iIncomingRequests[i].iPieceIndex == pieceIndex)
											&& (iIncomingRequests[i].iBegin == begin)
											&& (iIncomingRequests[i].iLength == length))
										{
											iIncomingRequests.Remove(i);
											break;
										}
									}									
								}
							}
							break;
							
							default:
								LogL(_L("?UNKNOWN? message received"));
							break;
						}						
					}
					
					iRecvBuffer->Delete(0, 4 + messageLength);
					
					if (iPeer->IsLocal())
						Torrent().iLocalBytesDownloaded += 4 + (messageLength);
					else
						Torrent().iLongRangeBytesDownloaded += 4 + (messageLength);
					
					// exit message processing loop if the peer is closing
					if (iState == EPeerClosing)
					{
						if (iPeer->IsLocal())
							Torrent().iLocalBytesDownloaded += iRecvBuffer->Size();
						else
							Torrent().iLongRangeBytesDownloaded += iRecvBuffer->Size();
						
						iRecvBuffer->Delete(0, iRecvBuffer->Size());
						
						break;
					}
				}
				else
					break; // from while
			}			
		}
		break;
		
		default:;
	}
	
	HLWRITELN(iLog, _L("[PeerConn] OnReceiveL end"));
}

void CSTPeerConnection::IssueUploadL()
{
	HLWRITELN(iLog, _L("[PeerConn] IssueUploadL begin"));
	
	if (!IsChoking() && !iUploadingPiece)
		if (iIncomingRequests.Count() > 0)
		{
			TSTBlockRequest& block = iIncomingRequests[0];
			SendPieceMessageL(block.iPieceIndex, block.iBegin, block.iLength);
			iIncomingRequests.Remove(0);			
		}
	
	HLWRITELN(iLog, _L("[PeerConn] IssueUploadL end"));
}


void CSTPeerConnection::IssueDownloadL()
{
	HLWRITELN(iLog, _L("[PeerConn] IssueDownloadL begin"));
	
	if (!iPeerWireConnected)
	{
		HLWRITELN(iLog, _L("[PeerConn] IssueDownloadL (!PWConn) end"));
		return;
	}
	
	HLWRITELN(iLog, _L("[PeerConn] IssueDownloadL before while"));
	
	TInt maxPieceRequests =  KMaxPieceRequests;
	if (iPeer->IsLocal())
		maxPieceRequests = KMaxPieceRequestsLocalConnection;
	
	while (iPiecesToDownload.Count() < maxPieceRequests)
	{
		CSTPiece* pieceToDownload = iTorrent->GetPieceToDownloadL(iPeer);
		if (pieceToDownload)
		{			
			CSTPieceToDownload* pieceToDlWrapper = 
				new (ELeave) CSTPieceToDownload(pieceToDownload, iEllapsedTime);
			CleanupStack::PushL(pieceToDlWrapper);
			iPiecesToDownload.AppendL(pieceToDlWrapper);
			CleanupStack::Pop(); // pieceToDlWrapper								
		}
		else
			break;			
	}
	
	if (iPiecesToDownload.Count() == 0)
	{
		SetInterestedL(EFalse);
		
		if (iEllapsedTime > 15)
		{
			if (!IsPeerInterested())
			{
				iPeer->SetPushPeer(EFalse);
				
				TConnectionCloseOrder order = EDelayReconnect;
				if (iTorrent->IsComplete())
					order = EDeletePeer;
				
				// local peers are disconnected only if the torrent is complete
				if ((iTorrent->IsComplete()) || (!iPeer->IsLocal()))
					CloseL(order, _L8("No pieces needed and peer is not interested"));
			}
//			else
//				if (iIncomingRequests.Count() == 0) // wait for possible incoming interested message
//				{										
//					CloseL(_L8("No pieces need and peer is not interested"));
//				}
		}			
	}
	else
	{
		SetInterestedL(ETrue);
		
		if (!IsPeerChoking())
		{
			for (TInt i=0; i<iPiecesToDownload.Count(); i++)
			{
				if (iTorrent->EndGame())
				{
					// during endgame, only one request per piece is allowed to
					if ((iPiecesToDownload[i]->iPendingRequests.Count() == 0) &&
						(iPiecesToDownload[i]->iPiece->DownloadedSize() < iPiecesToDownload[i]->iPiece->TotalSize()))
					{
						SendRequestMessageL(*iPiecesToDownload[i]);
					}
				}
				else
				{
					TInt maxPieceRequestQueueSize = KMaxPieceRequestQueueSize;
					if (iPeer->IsLocal())
						maxPieceRequestQueueSize = KMaxPieceRequestQueueSizeLocalConnection;
						
					while	((iPiecesToDownload[i]->iPendingRequests.Count() < maxPieceRequestQueueSize) &&
							(iPiecesToDownload[i]->iRequestedSize < iPiecesToDownload[i]->iPiece->TotalSize()))
					{
						SendRequestMessageL(*iPiecesToDownload[i]);
						//iPiecesToDownload[i].iHasPendingRequest = ETrue;
					}
				}				
			}			
		}
	}
	
/*	if (!iPieceToDownload)
	{
		iPieceToDownload = iTorrent->GetPieceToDownloadL(&iPeer);
		if (iPieceToDownload)
			iLastRequestTime = iEllapsedTime;
	}*/
		

/*	if (iPieceToDownload)
	{
		SetInterestedL(ETrue);		
		
		if (!IsPeerChoking())
		{
			if (!iHasPendingDownloadRequest)
			{
				SendRequestMessageL();
				iHasPendingDownloadRequest = ETrue;
			}			
		}
	}
	else
	{
		SetInterestedL(EFalse);	
		
		if ((iEllapsedTime > 15) && (iIncomingRequests.Count() == 0)) // wait for possible incoming interested message
				CloseL(_L8("No needed pieces and peer is not interested"));
	}	*/
	
	HLWRITELN(iLog, _L("[PeerConn] IssueDownloadL end"));
}


TUint CSTPeerConnection::ReadInt(TInt aIndex)
{
	TPtrC8 ptr = iRecvBuffer->Ptr().Right(
		iRecvBuffer->Ptr().Length() - aIndex);
	
	TUint value = ptr[0] << 24;
	value += (ptr[1] << 16);
	value += (ptr[2] << 8);
	value += ptr[3];
	
	return value;	
}


void CSTPeerConnection::OnTimerL()
{
	HLWRITELN(iLog, _L("[PeerConn] OnTimerL begin"));
	
	iEllapsedTime++;
	//if (iReconnectAfter > 0) iReconnectAfter--;		

	switch (iState)
	{		
		case EPeerTcpConnecting:
		{
			if (iEllapsedTime > KTcpConnectTimeout)
			{			
				CloseL(EIncreaseErrorCounter, _L8("Connection timeout"));
			}
		}
		break;
		
		case EPeerPwHandshaking:
		{			
			if (iEllapsedTime > KHandshakeTimeout)
			{
				HLWRITELN(iLog, _L("[PeerConn] RecvBuffer:"));
				HLWRITELN(iLog, iRecvBuffer->Ptr());
				CloseL(EIncreaseErrorCounter, _L8("Handshake timeout (no data received)"));
			}
		}
		break;
		
		case EPeerPwConnected:
		{
			if ((iEllapsedTime - iLastMessageSentTime) >= KKeepAliveInterval)
				SendKeepAliveMessageL();
			
			if ((iEllapsedTime - iLastMessageReceivedTime) > KPwConnectionTimeout)
			{
				CloseL(EIncreaseErrorCounter, _L8("General timeout (no data received)"));
				break;
			}
			
			// checking for timeouted requests
			if (iPeer->IsLocal() && (iNetMgr->Connection(1).Type() == CNetworkConnection::ERConnectionBased))
			{
				TBool requestTimeoutHappened = EFalse;
				TBool issueDownload = EFalse;
				
				for (TInt i=0; i<iPiecesToDownload.Count(); i++)
				{
					if (iPiecesToDownload[i]->iPendingRequests.Count() > 0)
					{
						issueDownload = ETrue;
						
						if ((iEllapsedTime - iPiecesToDownload[i]->iPendingRequests[0].iTimeOfSending) > KLocalRequestTimeoutUDP)
						{
							#ifdef LOG_TO_FILE
								TBuf<128> logEntry;
								logEntry.Format(_L("REQUEST[%d] TIMEOUT (begin: %d length: %d)"), iPiecesToDownload[i]->iPiece->Index(), 
										iPiecesToDownload[i]->iPendingRequests[0].iBegin, iPiecesToDownload[i]->iPendingRequests[0].iEnd - iPiecesToDownload[i]->iPendingRequests[0].iBegin);
								LogL(logEntry);
							#endif
								
							Torrent().iLocalRequestTimeouts += iPiecesToDownload[i]->iPendingRequests.Count();
								
							iPiecesToDownload[i]->iPendingRequests.Reset();
							iPiecesToDownload[i]->iRequestedSize =
								iPiecesToDownload[i]->iPiece->DownloadedSize();
							
							requestTimeoutHappened = ETrue;												
						}
					}
				}
				
				if (requestTimeoutHappened || issueDownload)
					IssueDownloadL();
			}
			else
			{
				if (iLastRequestTime && ((iEllapsedTime - iLastRequestTime) > KRequestTimeout))
				{
					iLastRequestTime = iEllapsedTime;
					if (Torrent().HasTimeoutlessPeer())
					{
						iPeer->SetHadRequestTimeout(ETrue);
						CloseL(EIncreaseErrorCounter, _L8("Request timeout"));
						break;
					}
				}
			}
			
			/*#ifdef LOG_TO_FILE
			if (iTorrent->IsComplete())
			{
				iLog->WriteL(_L("[PeerConn] Status: "));
				
				if (IsInterested())
					iLog->WriteL(_L("Interested, "));
				else
					iLog->WriteL(_L("Not interested, "));
				
				if (IsPeerInterested())
					iLog->WriteL(_L("Peer interested, "));
				else
					iLog->WriteL(_L("Peer not interested, "));
				
				iLog->WriteLineL(iEllapsedTime);
			}
			#endif*/
			
			if ((iEllapsedTime > 10) && (!IsInterested()) && (!IsPeerInterested()) && 
					((!iPeer->IsLocal()) || (iTorrent->IsComplete())))
			{
				CloseL(EDelayReconnect, _L8("Nobody interested!"));
				break;
			}
		}
		break;
		
		default:;
	}
	
	HLWRITELN(iLog, _L("[PeerConn] OnTimerL end"));
}


void CSTPeerConnection::ChangeState(TPeerConnectionState aState)
{
	iState = aState;
	iEllapsedTime = 0;
	
	if (iState == EPeerPwConnected)
	{
		if (iPeer)
		{
			TTime now;
			now.HomeTime();
			iPeer->SetLastConnectTime(now);
		}
	}
}


void CSTPeerConnection::CloseL(const TDesC8& aReason)
{
	CloseL(ENotSpecified, aReason);
}


void CSTPeerConnection::CloseL(TConnectionCloseOrder aOrder, const TDesC8& aReason)
{
	HLWRITELN(iLog, _L("[PeerConn] CloseL begin"));
	
	#ifdef LOG_TO_FILE
		TBuf<130> logb;
		logb.Format(_L("Requests: %d responses: %d ratio: %d%%"), 
			iLocalSentRequestCount, iLocalRequestWithResponseCount, iLocalRequestWithResponseCount/(TReal)iLocalSentRequestCount*100);
		LogL(logb);
	#endif
	
	if (iState != EPeerClosing)
	{
		ChangeState(EPeerClosing);
		
		#ifdef LOG_TO_FILE
			TBuf<100> log;			
			log.Copy(aReason.Left(70));
			log.Insert(0, _L("Closing connection: "));
			LogL(log);
		#endif
			
		iCloseOrder = aOrder;
		
		// the contents of the receivebuffer is processed and emptied in OnRecieveL !!!
		
		/*if (iTorrent && iRecvBuffer && (iRecvBuffer->Size() > 0))
		{
			if (iPeer->IsLocal())
				Torrent().iLocalBytesDownloaded += iRecvBuffer->Size();
			else
				Torrent().iLongRangeBytesDownloaded += iRecvBuffer->Size();
			
			iRecvBuffer->Delete(0, iRecvBuffer->Size());
		}*/
	
		StopAll();
		CloseSocket();
		
//		// If neither of the peers is interested after 10 seconds
//		// then they don't have any pieces to download from each other
//		if (iPeerWireConnected && (iEllapsedTime > 10))
//		{
//			if (!IsInterested() && !IsPeerInterested())
//				iCloseOrder = EDeletePeer;
//			LogL(_L("Deleting peer (No pieces needed and peer is not interested)"));
//		}		

/*
		TBuf<20> address;
		iPeer->Address().Output(address);
		iLog->WriteL(_L("[Peer "));
		iLog->WriteL(address);
		iLog->WriteL(_L("] Closing connection: "));
		iLog->WriteLineL(aReason);*/

		if (iTorrent)
			iTorrent->PeerDisconnectedL(this->iPeer, iPeerWireConnected);

		iPeerWireConnected = EFalse;
		
		if (iIncomingConnection)
			TorrentMgr()->DecIncomingConnectionCountL();

		/*	if (iReconnectAfter <= 0)
			iReconnectAfter = KDefaultDelayBeforeReconnecting;*/
	}
	
	HLWRITELN(iLog, _L("[PeerConn] CloseL end"));
}

void CSTPeerConnection::OnSocketConnectSucceededL()
{
	LogL(_L("Socket connected"));
	ChangeState(EPeerPwHandshaking);
	StartReceiving();
	
	SendHandshakeMessageL();
}

void CSTPeerConnection::OnSocketConnectFailedL(TInt aReason)
{
	TBuf8<128> errorText;
	_LIT8(KErrorText, "TCP connect failed (%d) - increaing error counter");
	errorText.Format(KErrorText, aReason);
	CloseL(EIncreaseErrorCounter, errorText);
}

void CSTPeerConnection::HandleWriteErrorL()
{	
	if (iPeerWireConnected && (iEllapsedTime > 10))
	{
		if (!IsInterested() && !IsPeerInterested())
		{
			CloseL(EDelayReconnect, _L8("Socket write failed (nobody interested, delayed reconnect"));
			return;
		}				
	}
	
	CloseL(_L8("Socket write failed"));
}

void CSTPeerConnection::HandleReadErrorL()
{
	if (iPeer->IsLocal() && (iState == EPeerPwHandshaking))
	{
		CloseL(EDeletePeer, _L8("Local peer refused handshake, deleting"));
		return;
	}
	
	if (iPeerWireConnected && (iEllapsedTime > 10))
	{
		if (!IsInterested() && !IsPeerInterested())
		{
			CloseL(EDelayReconnect, _L8("Socket read failed (nobody interested, delayed reconnect"));
			return;
		}
	}
	
	#ifdef LOG_TO_FILE
	if (iState == EPeerPwHandshaking)
	{
		LogL(_L("Handshake failed, receive buffer:"));
		iLog->WriteLineL(iRecvBuffer->Ptr());
	}
	#endif
	
	CloseL(_L8("Socket read failed"));
}


void CSTPeerConnection::SetChokingL(TBool aChoking)
{
	if (aChoking)
	{		
		if (!IsChoking())
		{
			iStatusFlags |= KFlagAmChoking;
			
			iIncomingRequests.Reset();
			
			if (iState == EPeerPwConnected)
			{
				iIncomingRequests.Reset();
				SendChokeMessageL();
			}
		}
	}
	else
		if (IsChoking())
		{
			iStatusFlags &= (~KFlagAmChoking);
			
			if (iState == EPeerPwConnected)
			{
				SendUnchokeMessageL();
				IssueUploadL();
			}
		}
	
}	
	
void CSTPeerConnection::SetInterestedL(TBool aInterested)
{
	if (aInterested)
	{		
		if (!IsInterested())
		{
			iStatusFlags |= KFlagAmInterested;
			
			if (iState == EPeerPwConnected)
				SendInterestedMessageL();
		}
	}
	else
		if (IsInterested())
		{
			iStatusFlags &= (~KFlagAmInterested);
			
			if (iState == EPeerPwConnected)
			{
				SendNotInterestedMessageL();
				
				if ((!IsPeerInterested()) && (iEllapsedTime > 10) && (!iPeer->IsLocal()))
					CloseL(EDelayReconnect, _L8("Nobody interested (delayed reconnect)"));
			}
				
		}	
}


const TSockAddr& CSTPeerConnection::RemoteAddress() 
{
	return iPeer->Address();
}


void CSTPeerConnection::SendHandshakeMessageL()
{
	if (iTorrent)
	{
		LogL(_L("Sending handshake"));
		
//		TBuf8<255> log;

		PutByteToSendBufferL(KLitProtocolId().Length());
//		log.SetLength(1);
//		log[0] = TChar(KLitProtocolId().Length());
							
		PutToSendBufferL(KLitProtocolId);
//		log.Append(KLitProtocolId);
		
		if (iPeer->IsLocal())
		{
			
			_LIT8(KLitReserved1, "\0\0\0");
			_LIT8(KLitReserved2, "\0\0\0\0");
			PutToSendBufferL(KLitReserved1);
			PutToSendBufferL(TPtrC8(&KGridTorrentExtensionByte, 1));
			PutToSendBufferL(KLitReserved2);
		}
		else
		{
			_LIT8(KLitReserved, "\0\0\0\0\0\0\0\0");
			PutToSendBufferL(KLitReserved);	
		}
		
//		log.Append(KLitReserved);	
		
		PutToSendBufferL(iTorrent->InfoHash());	
//		log.Append(iTorrent->InfoHash());	
		
		PutToSendBufferL(TorrentMgr()->PeerId());
//		log.Append(TorrentMgr()->PeerId());
//		iLog->WriteLineL(log);
		
		SendNow();
		
		LogL(_L("Handshake sent"));
		
		//SendInterestedMessageL();
	}
	else
		LogL(_L("ERROR, torrent is not specified, cannot send handshake"));	
}

void CSTPeerConnection::PutByteToSendBufferL(TChar aByte)
{
	TBuf8<1> buf;
	buf.SetLength(1);
	buf[0] = aByte;
	
	PutToSendBufferL(buf);
}

void CSTPeerConnection::SendByteL(TChar aByte)
{	
	PutByteToSendBufferL(aByte);
	SendNow();
}

	
void CSTPeerConnection::SendIntL(TUint32 aInteger)
{	
	PutIntToSendBufferL(aInteger);
	SendNow();	
}

void CSTPeerConnection::PutIntToSendBufferL(TUint32 aInteger)
{
	TBuf8<4> buffer;
	
	NSTUtils::WriteInt32(buffer, aInteger);
	
	PutToSendBufferL(buffer);
}

void CSTPeerConnection::SendKeepAliveMessageL()
{
	iLastMessageSentTime = iEllapsedTime;
	SendIntL(0);
	LogL(_L("out KEEPALIVE"));	
}

void CSTPeerConnection::SendBitfieldMessageL()
{
	iLastMessageSentTime = iEllapsedTime;
	TPtrC8 bitfield = iTorrent->BitField()->Data();
	PutIntToSendBufferL(1 + bitfield.Length()); // message length
	PutByteToSendBufferL(KMessageIdBitfield);
	SendL(bitfield);
	LogL(_L("out BITFIELD"));
}

void CSTPeerConnection::SendLocalBitfieldMessageL(const TDesC8& aBitField)
{
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(1 + aBitField.Length()); // message length
	PutByteToSendBufferL(KMessageIdLocalBitfield);
	SendL(aBitField);
	LogL(_L("out LOCALBITFIELD"));
}

void CSTPeerConnection::SendLocalPeerMessageL(TUint32 aAddress, TUint aPort)
{
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(9);
	PutByteToSendBufferL(KMessageIdLocalPeer);
	PutIntToSendBufferL(aAddress);
	PutIntToSendBufferL(aPort);
	LogL(_L("out LOCALPEER"));
}

void CSTPeerConnection::SendInterestedMessageL()
{	
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(1);
	SendByteL(KMessageIdInterested);	
	LogL(_L("out INTERESTED"));
}

void CSTPeerConnection::SendNotInterestedMessageL()
{	
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(1);
	SendByteL(KMessageIdNotInterested);	
	LogL(_L("out NOTINTERESTED"));
}

void CSTPeerConnection::SendChokeMessageL()
{
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(1);
	SendByteL(KMessageIdChoke);	
	LogL(_L("out CHOKE"));
}

void CSTPeerConnection::SendUnchokeMessageL()
{
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(1);
	SendByteL(KMessageIdUnchoke);	
	LogL(_L("out UNCHOKE"));
}

void CSTPeerConnection::SendRequestMessageL(CSTPieceToDownload& aPiece)
{
	HLWRITELN(iLog, _L("[PeerConn] SendRequestMessageL begin"));
	
	TInt begin = 0;
	if (iTorrent->EndGame())
		begin = aPiece.iPiece->DownloadedSize();
	else
		begin = aPiece.iRequestedSize;
	
	TInt maxBlockSize = KDefaultBlockSize;
		
	if (iPeer->IsLocal() && (iNetMgr->Connection(1).Type() == CNetworkConnection::ERConnectionBased))
		maxBlockSize = KDefaultBlockSizeLocalConnectionUDP;
	
	//if (Preferences()->SubpieceSize() == CSTPreferences::ESameAsPieceSize)
	//	maxBlockSize = iTorrent->PieceLength();
	
	if (Preferences()->SubpieceSize() != 0)
		maxBlockSize = Preferences()->SubpieceSize();
	
	TInt newRequestedSize;
	TInt blockSize;
	
	aPiece.iPiece->GetNextBlock(begin, maxBlockSize, blockSize, newRequestedSize);
	
	if (blockSize <= 0)
	{
		HLWRITELN(iLog, _L("[PeerConn] PANIC KPanTryingToSendWrongBlockRequest"));
		User::Panic(KSymTorrentEnginePanic, KPanTryingToSendWrongBlockRequest);
	}
	
	aPiece.iRequestedSize = newRequestedSize;
	
	iLastRequestTime = iEllapsedTime;
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(13);
	PutByteToSendBufferL(KMessageIdRequest);
	PutIntToSendBufferL(aPiece.iPiece->Index());
	PutIntToSendBufferL(begin);
	
	PutIntToSendBufferL(blockSize);
	SendNow();
	
	aPiece.iPendingRequests.AppendL(
		CSTPieceToDownload::TPieceRequest(begin, begin + blockSize, iEllapsedTime));
	
	
//	aPiece.iLastRequestLength = blockSize;
//	aPiece.iLastRequestBegin = aPiece.iPiece->DownloadedSize();
	
	#ifdef LOG_TO_FILE
		TBuf<128> logEntry;
		logEntry.Format(_L("out REQUEST[%d] (begin: %d length: %d)"), aPiece.iPiece->Index(), begin, blockSize);
		LogL(logEntry);
		
		if (iPeer->IsLocal())
			iLocalSentRequestCount++;
	#endif
		
	if (iPeer->IsLocal())
		Torrent().iSentLocalRequests++;
		
	HLWRITELN(iLog, _L("[PeerConn] SendRequestMessageL end"));
}

void CSTPeerConnection::SendCancelMessageL(CSTPieceToDownload& aPiece)
{
	iLastMessageSentTime = iEllapsedTime;
	
	for (TInt i=aPiece.iPendingRequests.Count()-1; i>=0; i--)
	{
		PutIntToSendBufferL(13);
		PutByteToSendBufferL(KMessageIdCancel);	
		PutIntToSendBufferL(aPiece.iPiece->Index());
		PutIntToSendBufferL(aPiece.iPendingRequests[i].iBegin);
		PutIntToSendBufferL(aPiece.iPendingRequests[i].Length());
		SendNow();
		
		aPiece.iRequestedSize -= aPiece.iPendingRequests[i].Length();
		
		#ifdef LOG_TO_FILE
			TBuf<128> logEntry;
			logEntry.Format(_L("out CANCEL[%d] (block length: %d)"), aPiece.iPiece->Index(), aPiece.iPendingRequests[i].Length());
			LogL(logEntry);
		#endif
	}
	
	aPiece.iPendingRequests.Reset();
}

void CSTPeerConnection::SendHaveMessageL(TInt aPieceIndex)
{
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(5);
	PutByteToSendBufferL(KMessageIdHave);	
	SendIntL(aPieceIndex);
	
	#ifdef LOG_TO_FILE
		TBuf<64> status;
		status.Format(_L("out HAVE Piece: %d"), aPieceIndex);
		LogL(status);
	#endif
}

void CSTPeerConnection::SendLocalHaveMessageL(TInt aPieceIndex)
{
	iLastMessageSentTime = iEllapsedTime;
	PutIntToSendBufferL(5);
	PutByteToSendBufferL(KMessageIdLocalHave);	
	SendIntL(aPieceIndex);
	
	#ifdef LOG_TO_FILE
		TBuf<64> status;
		status.Format(_L("out LOCALHAVE Piece: %d"), aPieceIndex);
		LogL(status);
	#endif
}

void CSTPeerConnection::SendPieceMessageL(TInt aPieceIndex, TInt aBegin, TInt aLength)
{
	CSTPiece* piece = iTorrent->Piece(aPieceIndex);
	
	if (piece)
	{
	//	TBuf<100> status1;
	//	status1.Format(_L("Processing piece request [%d] Begin: %d Length: %d while piece totalsize: %d"), aPieceIndex, aBegin, aLength, piece->TotalSize());
	//	LogL(status1);
		
		if (aBegin + aLength > piece->TotalSize())
		{
			CloseL(_L8("Bad PIECE request (index is out of bounds)"));
			return;
		}
		
		HBufC8* block = piece->GetBlockL(aBegin, aLength);
		
		if (block == NULL)
		{
			CloseL(_L8("Failed to extract block of piece"));
			return;
		}
		
		CleanupStack::PushL(block);	
		PutIntToSendBufferL(9 + aLength);
		PutByteToSendBufferL(KMessageIdPiece);
		PutIntToSendBufferL(aPieceIndex);
		PutIntToSendBufferL(aBegin);
		SendL(*block, this); // passes observer pointer to get notification after the piece upload
		CleanupStack::PopAndDestroy(); // block
		
		iTorrent->UpdateBytesUploadedL(aLength);
		
		iLastMessageSentTime = iEllapsedTime;
		
		TBuf<70> status;
		status.Format(_L("out PIECE Index: %d Begin: %d Length: %d"), aPieceIndex, aBegin, aLength);
		LogL(status);
	}
	else
		CloseL(_L8("Bad PIECE index"));	
}

void CSTPeerConnection::SetPeerWireConnectedL()
{
	iPeerWireConnected = ETrue;
	if (iPeer->IsLocal())
		iTorrent->IncreaseLocalConnectionCountL();
	else
		iTorrent->IncreaseConnectionCountL();
}

void CSTPeerConnection::SetPeerInterestedL(TBool aInterested) {
	if (aInterested)
		iStatusFlags |= KFlagPeerInterested;
	else
	{
		iStatusFlags &= (~KFlagPeerInterested);	
		
		if ((iState == EPeerPwConnected) && (!IsInterested()) && 
			((!iPeer->IsLocal()) || iTorrent->IsComplete()))
			CloseL(_L8("Nobody interested"));
	}
	
}

CSTTorrentManager* CSTPeerConnection::TorrentMgr()
{
	return iTorrentMgr;
}
	
CSTPreferences* CSTPeerConnection::Preferences()
{
	return iTorrentMgr->Preferences();	
}

TBool CSTPeerConnection::IsPushPeer() const {
	return iPeer->IsPushPeer();
}

void CSTPeerConnection::CancelPieceRequestL(CSTPiece* aPiece)
{
	HLWRITELN(iLog, _L("[PeerConn] CancelPieceRequestL begin"));
	
	if (iState == EPeerPwConnected)
	{
		for (TInt i=0; i<iPiecesToDownload.Count(); i++)
			if (iPiecesToDownload[i]->iPiece == aPiece)
			{
				if (iPiecesToDownload[i]->iPendingRequests.Count() > 0)
					SendCancelMessageL(*iPiecesToDownload[i]);
	
				delete iPiecesToDownload[i];
				iPiecesToDownload.Remove(i);
								
				break;
			}
	
		IssueDownloadL();
	}
	
	HLWRITELN(iLog, _L("[PeerConn] CancelPieceRequestL end"));
}

void CSTPeerConnection::CancelAllPieceRequestsL()
{
	HLWRITELN(iLog, _L("[PeerConn] CancelAllPieceRequestsL begin"));
	
	if (iState == EPeerPwConnected)
	{
		for (TInt i=0; i<iPiecesToDownload.Count(); i++)
			if (iPiecesToDownload[i]->iPendingRequests.Count() > 0)
				SendCancelMessageL(*iPiecesToDownload[i]);
		
		iPiecesToDownload.ResetAndDestroy();
		
		IssueDownloadL();
	}
	
	HLWRITELN(iLog, _L("[PeerConn] CancelAllPieceRequestsL end"));
}

void CSTPeerConnection::SetPeer(CSTPeer* aPeer)
{
	iPeer = aPeer;
}

void CSTPeerConnection::OnSocketWriteFinishedL()
{
	LogL(_L("Block uploaded successfully"));
	// this method is called only after a piece message has been sent
	iUploadingPiece = EFalse;
	IssueUploadL();
}

void CSTPeerConnection::OnPieceWriteFailedL(const CSTPiece& /*aPiece*/, TInt /*aBegin*/, TInt /*aLength*/)
{
	CloseL(_L8("Writing to piece failed")); // CRITICAL FAULT
}

void CSTPeerConnection::OnPieceWriteCompleteL(const CSTPiece& aPiece, TInt aBegin, TInt /*aLength*/)
{
	HLWRITELN(iLog, _L("[PeerConn] OnPieceWriteCompleteL begin"));
	
	TInt pieceArrayIndex = -1;
	for (TInt i=0; i<iPiecesToDownload.Count(); i++)
		if (iPiecesToDownload[i]->iPiece == &aPiece)
		{
			pieceArrayIndex = i;
			break;
		}
	
	if (pieceArrayIndex < 0)
	{
		LogL(_L("Error, writing piece complete but cannot find the entry"));
	}
	else
	{
		CSTPieceToDownload* pieceToDl = iPiecesToDownload[pieceArrayIndex];
		CSTPiece* piece = pieceToDl->iPiece;
		
		for (TInt i=0; i<pieceToDl->iPendingRequests.Count(); i++)
			if ((pieceToDl->iPendingRequests[i].iBegin == aBegin))
			{
				pieceToDl->iPendingRequests.Remove(i);										
				
				
				/// WHY??? (piece->Remaining() == piece->TotalSize())
				if ((piece->Remaining() == 0) || (piece->Remaining() == piece->TotalSize()))
				{
					delete iPiecesToDownload[pieceArrayIndex];
					iPiecesToDownload.Remove(pieceArrayIndex);
				}
				
				if (iTorrent->EndGame())									
					iTorrent->EndGamePieceReceivedL(piece, iPeer);
				
				IssueDownloadL();	
				break;
			}
	}
	
	HLWRITELN(iLog, _L("[PeerConn] OnPieceWriteCompleteL end"));
}

void CSTPeerConnection::SetPeerChoking(TBool aChoking) 
{
	if (aChoking)
		iStatusFlags |= KFlagPeerChoking;
	else
	{
		iLastRequestTime = 0;
		iStatusFlags &= (~KFlagPeerChoking);
	}
}

void CSTPeerConnection::HandlePieceMessageL(TInt aIndex, TInt aBegin, const TDesC8& aData)
{
	HLWRITELN(iLog, _L("[PeerConn] HandlePieceMessageL begin"));
	
	TInt pieceArrayIndex = -1;
	for (TInt i=0; i<iPiecesToDownload.Count(); i++)
		if (iPiecesToDownload[i]->iPiece->Index() == aIndex)
		{
			pieceArrayIndex = i;
			break;
		}
		
	if (pieceArrayIndex < 0)
	{
		//CloseL(_L8("Error, unexpected piece (there are no pending request for the received piece index)"));
		LogL(_L("Error, unexpected piece (there are no pending request for the received piece index)"));														
	}
	else
	{
		CSTPieceToDownload* pieceToDl = iPiecesToDownload[pieceArrayIndex];
		CSTPiece* piece = pieceToDl->iPiece;
		
		TBool foundPendingRequest = EFalse;
		
		for (TInt i=0; i<pieceToDl->iPendingRequests.Count(); i++)
		{
			if ((pieceToDl->iPendingRequests[i].iBegin == aBegin) &&
				(pieceToDl->iPendingRequests[i].Length() == aData.Length()))
			{
				foundPendingRequest = ETrue;
				pieceToDl->iPendingRequests.Remove(i);
				
				if (piece->InsertBlockL(aBegin, aData, iPeer) != KErrNone)
				{
					CloseL(_L8("Writing to piece failed")); // CRITICAL FAULT
					iTorrent->ReportCriticalFaultL(EWritingToDiskFailed);
				}
				else
				{
					// piece is complete or denied
					if (piece->Remaining() == 0 || 
						iPeer->FailedPieceCollector().IsDenied(*piece))
					{
						delete iPiecesToDownload[pieceArrayIndex];
						iPiecesToDownload.Remove(pieceArrayIndex);
					}
					else											
						if (iTorrent->EndGame())									
							iTorrent->EndGamePieceReceivedL(piece, iPeer);
				}
				
				break;
			}
		}
		
		if (!foundPendingRequest)
		{
			if (pieceToDl->iPendingRequests.Count() > 0)
			{
				#ifdef LOG_TO_FILE
					CSTPieceToDownload* pieceToDl = iPiecesToDownload[pieceArrayIndex];
					TBuf<200> info;
					info.Format(_L("Expected piece: begin: %d size: %d"), pieceToDl->iPendingRequests[0].iBegin, pieceToDl->iPendingRequests[0].Length());
					LogL(info);
				#endif
					
				//pieceToDl->iPendingRequests.Remove(0);
				pieceToDl->iPendingRequests.Reset();
				pieceToDl->iRequestedSize = piece->DownloadedSize();
			}
			//CloseL(_L8("Error, unexpected piece (pending request does not match)"));
			LogL(_L("Error, unexpected piece (pending request does not match)"));
		}
		
		
	/*	if ((pieceToDl->iPendingRequests.Count() > 0) &&
			(pieceToDl->iPendingRequests[0].iBegin == aBegin) &&
			(pieceToDl->iPendingRequests[0].Length() == aData.Length()) &&
			piece->DownloadedSize() == aBegin)
		{
			pieceToDl->iPendingRequests.Remove(0);										
			
			if (piece->AppendBlockL(aData, iPeer) != KErrNone)
			{
				CloseL(_L8("Writing to piece failed")); // CRITICAL FAULT
				iTorrent->ReportCriticalFaultL(EWritingToDiskFailed);
			}
			else
			{
				/// WHY??? (piece->Remaining() == piece->TotalSize())
				if (piece->Remaining() == 0)
				{
					delete iPiecesToDownload[pieceArrayIndex];
					iPiecesToDownload.Remove(pieceArrayIndex);
				}
				else											
					if (iTorrent->EndGame())									
						iTorrent->EndGamePieceReceivedL(piece, iPeer);
			}							
		}
		else
		{	
			if (pieceToDl->iPendingRequests.Count() > 0)
			{
				#ifdef LOG_TO_FILE
					CSTPieceToDownload* pieceToDl = iPiecesToDownload[pieceArrayIndex];
					TBuf<200> info;
					info.Format(_L("Expected piece: begin: %d size: %d"), pieceToDl->iPendingRequests[0].iBegin, pieceToDl->iPendingRequests[0].Length());
					LogL(info);
				#endif
					
				//pieceToDl->iPendingRequests.Remove(0);
				pieceToDl->iPendingRequests.Reset();
				pieceToDl->iRequestedSize = piece->DownloadedSize();
			}
			//CloseL(_L8("Error, unexpected piece (pending request does not match)"));
			LogL(_L("Error, unexpected piece (pending request does not match)"));																				
		}	*/																											
	}															
	
	IssueDownloadL();
	
	/*	if (iHasPendingDownloadRequest)
	{
		iHasPendingDownloadRequest = EFalse;

		if (index != iPieceToDownload->Index())
		{
			CloseL(EIncreaseErrorCounter, _L8("Piece index doesn't match")); // CRITICAL FAULT
			break;
		}

		if (iPieceToDownload->AppendBlockL(recv.Mid(13, messageLength - 9)) != KErrNone)
		{
			CloseL(_L8("Writing to piece failed")); // CRITICAL FAULT
		}

		if ((iPieceToDownload->Remaining() == 0) ||
			(iPieceToDownload->Remaining() == iPieceToDownload->TotalSize())) 
			iPieceToDownload = NULL;
		
	}
	else
		LogL(_L("Error, unexpected piece (there are no pending requests)"));*/
	
	HLWRITELN(iLog, _L("[PeerConn] HandlePieceMessageL end"));
}

TBool CSTPeerConnection::HandleIncomingLocalPieceL(TInt aIndex, TInt aBegin, const TDesC8& aData)
{
	HLWRITE(iLog, _L("[PeerConn] HandleIncomingLocalPieceL, index: "));
	HLWRITE(iLog, aIndex);
	HLWRITE(iLog, _L(" begin: "));
	HLWRITE(iLog, aBegin);
	HLWRITE(iLog, _L(" length: "));
	HLWRITELN(iLog, aData.Length());
	
	HLWRITELN(iLog, _L("[PeerConn] Pieces to download: "));
	TInt pieceArrayIndex = -1;
	for (TInt i=0; i<iPiecesToDownload.Count(); i++)
	{
		HLWRITELN(iLog, iPiecesToDownload[i]->iPiece->Index());
		
		if (iPiecesToDownload[i]->iPiece->Index() == aIndex)
		{
			pieceArrayIndex = i;
			break;
		}
	}
		
	if (pieceArrayIndex >= 0)
	{
		CSTPieceToDownload* pieceToDl = iPiecesToDownload[pieceArrayIndex];
		
		for (TInt i=0; i<pieceToDl->iPendingRequests.Count(); i++)
		{
			if ((pieceToDl->iPendingRequests[i].iBegin == aBegin) &&
				(pieceToDl->iPendingRequests[i].Length() == aData.Length()))
			{
				#ifdef LOG_TO_FILE
					iLocalRequestWithResponseCount++;
				#endif
				HandlePieceMessageL(aIndex, aBegin, aData);
				
				Torrent().iLocalSubPiecesForRequests++;
				Torrent().iLocalSubPiecesForRequestsSize += aData.Size();
				
				return ETrue;
			}
		}
		
		LWRITELN(iLog, _L("[PeerConn] Unexpected piece (begin does not match)!"));
	}
	else
	{
		LWRITELN(iLog, _L("[PeerConn] Unexpected piece (piece index not found)!"));
	}
	
	return EFalse;
}

void CSTPeerConnection::OnDataSentL(TInt aSize)
{
	if (iPeer->IsLocal())
		Torrent().iLocalBytesUploaded += aSize;
	else
		Torrent().iLongRangeBytesUploaded += aSize;
}
