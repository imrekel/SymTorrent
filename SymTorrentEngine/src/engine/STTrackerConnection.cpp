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

/* ============================================================================
 *  Name     : CSTTrackerConnection from STTrackerConnection.cpp
 *  Part of  : SymTorrent
 *  Created  : 14.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STTrackerConnection.h"
#include "STTorrent.h"
#include "SymTorrentEngineLog.h"
#include "STTorrentManager.h"
#include "STPreferences.h"
#include "STUtil.h"
#include "STBencode.h"
#include "STAnnounceList.h"
#include <uri8.h>
#include <http.h>
#include <chttpformencoder.h>
#include <HttpStringConstants.h>
#include <http\RHTTPTransaction.h>
#include <http\RHTTPSession.h>
#include <http\RHTTPHeaders.h>
#include <http\thttphdrfielditer.h>
#include <EscapeUtils.h>

const TInt KMaxTrackerAnnounceTime = 25; // 25 seconds

// ================= MEMBER FUNCTIONS =======================


// Used user agent for requests
_LIT8(KUserAgent, "SymTorrent");

// This client accepts all content types.
// (change to e.g. "text/plain" for plain text only)
_LIT8(KAccept, "*/*");


CSTTrackerConnection::CSTTrackerConnection(CSTTorrent& aTorrent, TTrackerConnectionEvent aEvent)
 : iTorrent(aTorrent), iEvent(aEvent)
{
	iNetMgr = iTorrent.TorrentMgr()->NetworkManager();
}


CSTTrackerConnection::~CSTTrackerConnection()
{
	delete iConnectionTimeout;
	delete iAddress;
	
	if (iRunning)
	{
		iTransaction.Close();		
	}	
	
	iNetMgr->Close(iSession);
	
	delete iUri;	
	delete iReceiveBuffer;
}


void CSTTrackerConnection::ConstructL(const TDesC8& aTrackerAddess)
{
	#ifdef LOG_TO_FILE
	iLog = LOGMGR->GetLoggerL(TUid::Uid(SYMTORRENT_ENGINE_UID));
	#endif
	
	iAddress = aTrackerAddess.AllocL();
	iConnectionTimeout = CTimeout::NewL(this);
	
	/*// Open RHTTPSession with default protocol ("HTTP/TCP")
	TRAPD(err, iSession.OpenL());
	if(err != KErrNone) 
	{
		// Most common error; no access point configured, and session creation
		// leaves with KErrNotFound.
		_LIT(KErrMsg,
			"[Trackerconnection] Cannot create session. Is internet access point configured?");
		_LIT(KExitingApp, "Exiting app.");
		//CEikonEnv::Static()->InfoWinL(KErrMsg, KExitingApp);
		User::Leave(err);
	}*/
}

void CSTTrackerConnection::HTTPSessionOpenedL(TBool aResult, RHTTPSession& /*aHTTPSession*/)
{
	if (aResult)
	{
		// starts listening before connecting to the tracker
		// TODO this should be called from the torrent / torrentmanager
		iTorrent.TorrentMgr()->NetworkManager()->StartListeningL(
				0, iTorrent.Preferences()->IncomingPort());
		
		CreateUriL();
		
		// Parse string to URI (as defined in RFC2396)
		TUriParser8 uri;
		uri.Parse(*iUri);

		// Get request method string for HTTP GET
		RStringF method = iSession.StringPool().StringF(HTTP::EGET,
			RHTTPSession::GetTable());
			


		// Open transaction with previous method and parsed uri. This class will
		// receive transaction events in MHFRunL and MHFRunError.
		iTransaction = iSession.OpenTransactionL(uri, *this, method);

		// Set headers for request; user agent and accepted content type
		RHTTPHeaders hdr = iTransaction.Request().GetHeaderCollection();
		SetHeaderL(hdr, HTTP::EUserAgent, KUserAgent);
		SetHeaderL(hdr, HTTP::EAccept, KAccept);

		// Submit the transaction. After this the framework will give transaction
		// events via MHFRunL and MHFRunError.
		LWRITE(iLog, _L("[Trackerconnection] Starting transaction: GET "));
		LWRITELN(iLog, *iUri);
		iTransaction.SubmitL();

		iRunning = ETrue;
		iConnectionTimeout->Reset(KMaxTrackerAnnounceTime * 1000);
	}
	else
	{
		LWRITE(iLog, _L("[Trackerconnection] Failed "));
		if (iObserver)
			iObserver->TrackerConnectionFailedL();
		
		/*iTorrent.SetFailedL(_L("Failed to establish network connection."));*/
	}
}

void CSTTrackerConnection::SetHeaderL(RHTTPHeaders aHeaders, 
							 TInt aHdrField, 
							 const TDesC8& aHdrValue)
{
	RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
	CleanupClosePushL(valStr);	
	THTTPHdrVal val(valStr);
	aHeaders.SetFieldL(iSession.StringPool().StringF(aHdrField,
		RHTTPSession::GetTable()), val);
	CleanupStack::PopAndDestroy(); // valStr
}


void CSTTrackerConnection::CreateUriL()
{
	TInetAddr localAddress;
	TInt getAddressRes = KErrGeneral;
	// only send IP if we are connected via proxy
	//if (Preferences()->IncomingConnectionsMode() == EEnabledWithProxy)
	//	getAddressRes = iNetMgr->Address(localAddress);
	
	CBufFlat* uriBuf = CBufFlat::NewL(512);
	CleanupStack::PushL(uriBuf);
	
	//TPtrC8 activeTracker = iTorrent.AnnounceList()->ActiveAddress();
	//LWRITE(iLog, _L("Active tracker: "));
	//LWRITELN(iLog, activeTracker);
	
	uriBuf->InsertL(uriBuf->Size(), *iAddress);
	uriBuf->InsertL(uriBuf->Size(), _L8("?"));
	
	uriBuf->InsertL(uriBuf->Size(), _L8("info_hash="));	
	HBufC8* encoded = EscapeUtils::EscapeEncodeL(iTorrent.InfoHash(), EscapeUtils::EEscapeUrlEncoded);
	CleanupStack::PushL(encoded);
	uriBuf->InsertL(uriBuf->Size(), *encoded);
	CleanupStack::PopAndDestroy(); // encoded
	
	uriBuf->InsertL(uriBuf->Size(), _L8("&peer_id="));
	encoded = EscapeUtils::EscapeEncodeL(TorrentMgr()->PeerId(), EscapeUtils::EEscapeUrlEncoded);
	CleanupStack::PushL(encoded);
	uriBuf->InsertL(uriBuf->Size(), *encoded);
	CleanupStack::PopAndDestroy(); // encoded
	
	uriBuf->InsertL(uriBuf->Size(), _L8("&key="));
	TBuf8<32> keyBuf;
	keyBuf.Num(TorrentMgr()->Key());
	uriBuf->InsertL(uriBuf->Size(), keyBuf);
	
	uriBuf->InsertL(uriBuf->Size(), _L8("&port="));
	TBuf8<32> portBuf;
	if (getAddressRes == KErrNone)
		portBuf.Num(localAddress.Port());
	else
		portBuf.Num(Preferences()->IncomingPort());
	uriBuf->InsertL(uriBuf->Size(), portBuf);
	
	uriBuf->InsertL(uriBuf->Size(), _L8("&uploaded="));
	TBuf8<24> bytesUploaded;
	bytesUploaded.Num(iTorrent.BytesUploaded());
	uriBuf->InsertL(uriBuf->Size(), bytesUploaded);
	
	uriBuf->InsertL(uriBuf->Size(), _L8("&downloaded="));
	TBuf8<24> bytesDownloaded;
	bytesDownloaded.Num(iTorrent.BytesDownloaded());
	uriBuf->InsertL(uriBuf->Size(), bytesDownloaded);
	
	uriBuf->InsertL(uriBuf->Size(), _L8("&left="));
	TBuf8<24> bytesLeft;
	bytesLeft.Num(iTorrent.BytesLeft());
	uriBuf->InsertL(uriBuf->Size(), bytesLeft);

	// it seems that some trackers support only compact responses
	uriBuf->InsertL(uriBuf->Size(), _L8("&compact=1"));

	if (iEvent != ETrackerEventNotSpecified)
	{
		uriBuf->InsertL(uriBuf->Size(), _L8("&event="));
		
		switch (iEvent)
		{
			case ETrackerEventStarted:
				uriBuf->InsertL(uriBuf->Size(), _L8("started"));
			break;
			
			case ETrackerEventStopped:
				uriBuf->InsertL(uriBuf->Size(), _L8("stopped"));
			break;
			
			case ETrackerEventCompleted:
				uriBuf->InsertL(uriBuf->Size(), _L8("completed"));
			break;
			
			default:
			break;
		}
	}
	
	if (getAddressRes == KErrNone)
	{
		TBuf<64> ipBuf;
		localAddress.Output(ipBuf);
		TBuf8<64> ipBuf8;
		ipBuf8.Copy(ipBuf);
		
		uriBuf->InsertL(uriBuf->Size(), _L8("&ip="));
		uriBuf->InsertL(uriBuf->Size(), ipBuf8);
		
		#ifdef LOG_TO_FILE
		// debug info
		LWRITE(iLog, _L("Sent to tracker: "));
		LWRITE(iLog, ipBuf);
		TBuf8<32> portBuf;
		portBuf.Num(localAddress.Port());
		LWRITE(iLog, _L(":"));
		LWRITELN(iLog, portBuf);
		#endif
		//		
	}
	
	iUri = uriBuf->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(); // uriBuf
	
	//iLog->WriteL(_L("[Trackerconnection] GET "));
	//iLog->WriteLineL(*iUri);
}


void CSTTrackerConnection::StartTransactionL()
{
	LWRITELN(iLog, _L("[Trackerconnection] starting transaction"));
	
	iNetMgr->Close(iSession);
	iNetMgr->OpenHTTPSessionL(iSession, this, 0);
}

void CSTTrackerConnection::Cancel()
{
	if(!iRunning) 
		return;

	// Close() also cancels transaction (Cancel() can also be used but 
	// resources allocated by transaction must be still freed with Close())
	iTransaction.Cancel();
	iTransaction.Close();

	// Not running anymore
	iRunning = EFalse;
}


void CSTTrackerConnection::MHFRunL(RHTTPTransaction aTransaction, 
						  			const THTTPEvent& aEvent)
{
	switch (aEvent.iStatus)
	{
		case THTTPEvent::EGotResponseHeaders:
		{
			// HTTP response headers have been received. Use
			// aTransaction.Response() to get the response. However, it's not
			// necessary to do anything with the response when this event occurs.

			LWRITELN(iLog, _L("[Trackerconnection] Got HTTP headers"));
			// Get HTTP status code from header (e.g. 200)
			RHTTPResponse resp = aTransaction.Response();
			TInt status = resp.StatusCode();
			
			if (status != 200) // ERROR, hiba esetén mi legyen? 404-et lekezelni!
			{
				LWRITE(iLog, _L("[Trackerconnection] Error, status = "));
				TBuf<20> numBuf;
				numBuf.Num(status);
				LWRITELN(iLog, numBuf);
				Cancel();
				if (iObserver)
					iObserver->TrackerConnectionFailedL();
				break;
			}

			// Get status text (e.g. "OK")
			HLWRITE(iLog, _L("[Trackerconnection] Status text = "));
			TBuf<32> statusText;
			statusText.Copy(resp.StatusText().DesC());
			HLWRITELN(iLog, statusText);
			
			
			#ifdef LOG_TO_FILE
			RHTTPHeaders headers = 
				aTransaction.Response().GetHeaderCollection();		
			THTTPHdrFieldIter i =
				headers.Fields();
			for (i.First(); !(i.AtEnd()); ++i)
			{
				RStringF header = iSession.StringPool().StringF(i());
				
				if ((header.DesC() == _L8("Content-Type")))
				{
					HLWRITE(iLog, header.DesC());
					HLWRITE(iLog, _L(": "));
					THTTPHdrVal	val;
					headers.GetField(header, 0, val);
					RStringF value = val.StrF();
					HLWRITELN(iLog, value.DesC());
				}
				else
					HLWRITELN(iLog, header.DesC());
			}
						
			#endif
		}
		break;

		case THTTPEvent::EGotResponseBodyData:
		{			
			// Part (or all) of response's body data received. Use 
			// aTransaction.Response().Body()->GetNextDataPart() to get the actual
			// body data.						

			// Get the body data supplier
			MHTTPDataSupplier* body = aTransaction.Response().Body();
			TPtrC8 dataChunk;						

			// GetNextDataPart() returns ETrue, if the received part is the last 
			// one.
			TBool isLast = body->GetNextDataPart(dataChunk);
			
			//iDownloadedSize += dataChunk.Size();						
			
			HLWRITELN(iLog, _L8("[TrackerConnection] HTTP response body chunk received: "));
			HLWRITELN(iLog, dataChunk);
			
			if (iReceiveBuffer)
			{
				HBufC8* temp = HBufC8::NewL(
					iReceiveBuffer->Length() + dataChunk.Length());
				TPtr8 tempPtr(temp->Des());
				tempPtr.Copy(*iReceiveBuffer);
				tempPtr.Append(dataChunk);
				
				delete iReceiveBuffer;
				iReceiveBuffer = temp;
			}
			else
				iReceiveBuffer = dataChunk.AllocL();

			// Always remember to release the body data.
			body->ReleaseData();
		
			// NOTE: isLast may not be ETrue even if last data part received.
			// (e.g. multipart response without content length field)
			// Use EResponseComplete to reliably determine when body is completely
			// received.
			if (isLast)
			{
				
				#ifdef LOG_TO_FILE
				_LIT(KBodyReceived,"Body received");
				HLWRITELN(iLog, KBodyReceived);
				#endif
				
				//CSTBencode* bencodedResponse = CSTBencode::ParseL(*iReceiveBuffer);	
				//iLog->WriteLineL(*iReceiveBuffer);
				//
				//if (bencodedResponse)
				//{
				//	CleanupStack::PushL(bencodedResponse);
				//	iTorrent.ProcessTrackerResponseL(bencodedResponse);
				//	CleanupStack::PopAndDestroy(); // bencodedResponse
				//}
			}
		}
		break;

		case THTTPEvent::EResponseComplete:
		{
			// Indicates that header & body of response is completely received.
			// No further action here needed.
			//_LIT(KTransactionComplete, "Transaction Complete");
			//iLog->WriteLineL(KTransactionComplete);
			//iResult = ESucceeded;
		}
		break;

		case THTTPEvent::ESucceeded:
		{
			LWRITELN(iLog, _L("[Trackerconnection] HTTP transaction succeded"));

			CSTBencode* bencodedResponse = CSTBencode::ParseL(*iReceiveBuffer);	
			//iLog->WriteLineL(*iReceiveBuffer);
			
			if (bencodedResponse && iObserver)
			{
				CleanupStack::PushL(bencodedResponse);
				iObserver->TrackerResponseReceivedL(*bencodedResponse);
				CleanupStack::PopAndDestroy(); // bencodedResponse
			}
			
			iRunning = EFalse;
			
			if (iObserver)
				iObserver->TrackerConnectionSucceededL();
		}
		break;

		case THTTPEvent::EFailed:
		{
			LWRITELN(iLog, _L("[Trackerconnection] HTTP transaction failed"));
			iRunning = EFalse;
			if (iObserver)
				iObserver->TrackerConnectionFailedL();
		}
		break;

		default:
			// There are more events in THTTPEvent, but they are not usually 
			// needed. However, event status smaller than zero should be handled 
			// correctly since it's error.
		{
			TBuf<64> text;
			if (aEvent.iStatus < 0)
			{
				LWRITE(iLog, _L("[Trackerconnection] HTTP transaction failed, "));
				_LIT(KErrorStr, "Error: %d");
				text.Format(KErrorStr, aEvent.iStatus);
				LWRITELN(iLog, text);
			
				// Just close the transaction on errors
				aTransaction.Close();
				iRunning = EFalse;
				
				if (iObserver)
					iObserver->TrackerConnectionFailedL();
			}
			else
			{
				// Other events are not errors (e.g. permanent and temporary
				// redirections)
				_LIT(KUnrecognisedEvent, "[Trackerconnection] Unrecognised event: %d");
				text.Format(KUnrecognisedEvent, aEvent.iStatus);
				LWRITELN(iLog, text);
			}		
		}
		break;
	}
}


TInt CSTTrackerConnection::MHFRunError(TInt /*aError*/, 
							  RHTTPTransaction /*aTransaction*/, 
							  const THTTPEvent& /*aEvent*/)
{
	// Just notify about the error and return KErrNone.
/*	TBuf<64>	text;
	_LIT(KRunError, "MHFRunError: %d");
	text.Format(KRunError, aError);
	iObserver.ClientEvent(text);*/
	return KErrNone;
}


CSTPreferences* CSTTrackerConnection::Preferences() 
{
	return iTorrent.Preferences();
}
	
	
CSTTorrentManager* CSTTrackerConnection::TorrentMgr() 
{
	return iTorrent.TorrentMgr();
}

TBool CSTTrackerConnection::HandleTimeIsUp(CTimeout* aTimeout, TInt aStatus)
{
	LWRITELN(iLog, _L("[Trackerconnection] Connection timeout"));		
	Cancel();
	
	if (iObserver)
		iObserver->TrackerConnectionFailedL();
	
	return EFalse;
}
