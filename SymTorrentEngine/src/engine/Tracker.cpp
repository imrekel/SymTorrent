/*
 ============================================================================
 Name		: Tracker.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CTracker implementation
 ============================================================================
 */

#include "Tracker.h"
#include "STTrackerConnection.h"
#include "STTorrent.h"
#include "STPeer.h"
#include "SymTorrentEngineLog.h"
#include "STTorrentManager.h"

const TInt KDefaultTrackerReconnectInterval = 900; // in seconds
const TInt KTrackerReconnectIntervalAfterFailure = 30; // in seconds

CTracker::CTracker(CTrackerManager& aTrackerManager)
 : iTrackerManager(aTrackerManager)
{
	// No implementation required
}

CTracker::~CTracker()
{
	delete iConnectTimeout;
	delete iConnection;
	delete iAddress;
	iTrackerManager.Torrent().TorrentMgr()->NetworkManager()->RemoveObserver(this);
}

void CTracker::ConstructL(const TDesC8& aAddress)
{
	iAddress = aAddress.AllocL();
	iConnectTimeout = CTimeout::NewL(this);
	
	iTrackerManager.Torrent().TorrentMgr()->NetworkManager()->AddObserverL(this);
}

TBool CTracker::HandleTimeIsUp(CTimeout* aTimeout, TInt aStatus)
{
	AnnounceL(iCurrentEvent);
	
	return EFalse;
}

void CTracker::AnnounceL(TTrackerConnectionEvent aEvent)
{
	LWRITE(LOG, _L("[CTracker] Starting to announce to tracker: "));
	LWRITELN(LOG, Address());
	
	if (iConnection)
	{
		delete iConnection;
		iConnection = NULL;
	}
	iConnectTimeout->Stop();
	
	iCurrentEvent = aEvent;
	
	iIsAnnouncing = ETrue;
	
	if (iTrackerManager.Torrent().TorrentMgr()->NetworkManager()->IsNetworkConnectionStarted(0))
	{
		iConnection = new (ELeave) CSTTrackerConnection(iTrackerManager.Torrent(), aEvent);
		iConnection->ConstructL(*iAddress);
		iConnection->SetObserver(this);
		iConnection->StartTransactionL();
	}
	else
	{
		iTrackerManager.Torrent().TorrentMgr()->NetworkManager()->StartNetworkConnectionL(0);
	}
}

void CTracker::Close()
{
	iIsAnnounced = EFalse;
	iIsAnnouncing = EFalse;
	delete iConnection;
	iConnection = NULL;
	iConnectTimeout->Stop();
}

void CTracker::TrackerConnectionFailedL()
{
	iConnectionFailureCount++;
	
	LWRITE(LOG, _L("[CTracker] Connection failed, retry count: "));
	LWRITE(LOG, iConnectionFailureCount);
	LWRITE(LOG, _L(" for tracker: "));
	LWRITELN(LOG, Address());
	
	if (iConnectionFailureCount < 3)
	{
		iConnectTimeout->Reset(KTrackerReconnectIntervalAfterFailure * 1000);
	}
	else
	{
		iIsAnnouncing = EFalse;
		iConnectionFailureCount = 0;
		
		if (iCurrentEvent == ETrackerEventStopped)
		{
			Close();
		}
		else
		{
			if (iTrackerInterval > 0 && iTrackerInterval > 60)
				iConnectTimeout->Reset(iTrackerInterval * 1000);
			else
				iConnectTimeout->Reset(KDefaultTrackerReconnectInterval * 1000);
		}
		
		if (iObserver)
			iObserver->TrackerAnnounceFinishedL(*this);
		
		iCurrentEvent = ETrackerEventNotSpecified;
	}
	
	delete iConnection;
	iConnection = NULL;
}

void CTracker::TrackerConnectionSucceededL()
{
	LWRITE(LOG, _L("[CTracker] Connection succeeded for tracker: "));
	LWRITELN(LOG, Address());
	
	iConnectionFailureCount = 0;
	iIsAnnouncing = EFalse;
	
	if (iCurrentEvent == ETrackerEventStopped) // stop reconnecting
	{
		Close();
	}
	else
	{
		iIsAnnounced = ETrue;
		
		if (iTrackerInterval > 0 && iTrackerInterval > 60)
			iConnectTimeout->Reset(iTrackerInterval * 1000);
		else
			iConnectTimeout->Reset(KDefaultTrackerReconnectInterval * 1000);
	}
	
	if (iObserver)
		iObserver->TrackerAnnounceFinishedL(*this);
	
	iCurrentEvent = ETrackerEventNotSpecified;
	
	delete iConnection;
	iConnection = NULL;
}

void CTracker::TrackerResponseReceivedL(CSTBencode& aResponse)
{		
	LWRITELN(LOG, _L("[CTracker] Response received, parsing..."));
	
	CSTTorrentManager* torrentMgr = iTrackerManager.Torrent().TorrentMgr();
	CSTTorrent& torrent = iTrackerManager.Torrent();
	
	TInetAddr localAddress;
	TInt getAddressRes = torrentMgr->NetworkManager()->GetLocalAddress(0, localAddress);
	
	if (aResponse.Type() != EBencodedDictionary)
	{
		LWRITELN(LOG, _L("[CTracker] Invalid response (not a bencoded dictionary)"));
		return;
	}
	
	CSTBencodedDictionary* response = 
		static_cast<CSTBencodedDictionary*>(&aResponse);
		
	CSTBencode* value = NULL;
	
	// interval
	{
		_LIT8(KLitInterval, "interval");
		value = response->EntryValue(KLitInterval);
		
		if (value && (value->Type() == EBencodedInteger))
		{
			TInt interval = static_cast<CSTBencodedInteger*>(value)->iValue;
			
			LWRITE(LOG, _L("[Announce] Interval parsed: "));
			LWRITELN(LOG, interval);
			
			if ((interval > 0) && (interval <= 3600))
				iTrackerInterval = interval;
		}
	}

	// failure reason
	{
		_LIT8(KLitFailureReason, "failure reason");
		value = response->EntryValue(KLitFailureReason);
		
		if (value && (value->Type() == EBencodedString))
		{
			LWRITE(LOG, _L("[Announce] Request failed, reason: "));
			LWRITELN(LOG, static_cast<CSTBencodedString*>(value)->Value());
		}
	}
	
	// complete
	{
	}
	
	// incomplete
	{		
	}
	
	// peers
	{
		LWRITE(LOG, _L("[Torrent] Local peer id: "));
		LWRITELN(LOG, torrentMgr->PeerId());
		
		_LIT8(KLitIp, "ip");
		_LIT8(KLitPeerId, "peer id");
		_LIT8(KLitPort, "port");
		
		_LIT8(KLitPeers, "peers");
		value = response->EntryValue(KLitPeers);
		
		if (value && (value->Type() == EBencodedList)) // normal tracker response
		{
			CSTBencodedList* peers = 
				static_cast<CSTBencodedList*>(value);
			
			LWRITE(LOG, _L("[Announce] Number of peers received: "));
			LWRITELN(LOG, peers->Count());
				
			for (TInt i=0; i<peers->Count(); i++)
			{
				value = peers->Item(i);
				
				if (value->Type() != EBencodedDictionary)
					return;
				
				CSTBencodedDictionary* bencodedPeer = 
					static_cast<CSTBencodedDictionary*>(value);
					
				// peer id
					value = bencodedPeer->EntryValue(KLitPeerId);
					if (!value || (value->Type() != EBencodedString))
						continue;
					TPtrC8 peerId = static_cast<CSTBencodedString*>(value)->Value();
					
					LWRITE(LOG, _L("[Announce] Processing peer id: "));
					LWRITELN(LOG, peerId);
					if (peerId.Length() != 20)
						continue;
					
					if (peerId.CompareF(torrentMgr->PeerId()) == 0) // got back our own address
					{
						LWRITELN(LOG, _L("[Announce] Got own local address from tracker (peer ID is the same), throwing it away..."));
						continue;
					}
				
				// ip
					value = bencodedPeer->EntryValue(KLitIp);
					if (!value || (value->Type() != EBencodedString))
						continue;
					TPtrC8 ip = static_cast<CSTBencodedString*>(value)->Value();
					
					HBufC* ip16 = HBufC::NewLC(ip.Length());
					TPtr ip16ptr(ip16->Des());
					ip16ptr.Copy(ip);
					
					LWRITE(LOG, _L("[Announce] Peer address received: "));
					LWRITE(LOG, ip);
					LWRITE(LOG, _L(":"));
					
					TInetAddr address;				
					if (address.Input(*ip16) != KErrNone)
					{
						CleanupStack::PopAndDestroy(); // ip16
						continue;						
					}									
					
					CleanupStack::Pop(); // ip16						
				
				// port
					value = bencodedPeer->EntryValue(KLitPort);
					if (!value || (value->Type() != EBencodedInteger))
						continue;
					address.SetPort(static_cast<CSTBencodedInteger*>(value)->iValue);
					
					LWRITE(LOG, static_cast<CSTBencodedInteger*>(value)->iValue);
					
					if (getAddressRes == KErrNone)
					{
						if (address == localAddress)
						{
							LWRITELN(LOG, _L("[Announce] Got own local address from tracker, throwing it away..."));
							continue;
						}						
					}								
					
				CSTPeer* peer = CSTPeer::NewLC(address, peerId, torrent.PieceCount());
				if (torrent.AddPeerL(peer) != KErrNone)
				{
					LWRITELN(LOG, _L(" NOT added"));
					CleanupStack::PopAndDestroy(); // peer
				}				
				else
				{
					LWRITELN(LOG, _L(" ADDED"));
					CleanupStack::Pop(); // peer
				}					
			}
		}
		else

		if (value && (value->Type() == EBencodedString)) // likely a compact response
		{
			CSTBencodedString* peers = 
				static_cast<CSTBencodedString*>(value);

			if ((peers->Value().Length() % 6) == 0)
			{
				TPtrC8 ips = peers->Value();
				for (TInt i = 0; i < ips.Length()/6; i++)
				{
					TInt pos = i * 6;
					TBuf<25> addressBuffer;
					_LIT(KLitIpFormat, "%u.%u.%u.%u");
					addressBuffer.Format(KLitIpFormat, TUint(ips[pos]), TUint(ips[pos+1]), 
						TUint(ips[pos+2]), TUint(ips[pos+3]));

					TInetAddr address;				
					if (address.Input(addressBuffer) != KErrNone)
						continue;

					address.SetPort(TUint((ips[pos+4] << 8) + ips[pos+5]));
					
					TBuf<32> ipBuf;
					address.Output(ipBuf);
					LWRITE(LOG, _L("[Announce] Peer address received: "));
					LWRITE(LOG, ipBuf);
					LWRITE(LOG, _L(":"));
					LWRITE(LOG, address.Port());
					
					if (getAddressRes == KErrNone)
					{
						if (address == localAddress)
						{						
							LWRITELN(LOG, _L(" NOT ADDED (Got own local address from tracker, throwing it away...)"));
							continue;
						}
					}

					CSTPeer* peer = CSTPeer::NewLC(address, torrent.PieceCount());
					if (torrent.AddPeerL(peer) != KErrNone)
					{
						LWRITELN(LOG, _L(" NOT added"));
						CleanupStack::PopAndDestroy(); // peer
					}
						
					else
					{
						LWRITELN(LOG, _L(" ADDED"));
						CleanupStack::Pop(); // peer
					}
						
				}
			}
			else
				LWRITELN(LOG, _L("[Announce] Compact response invalid (length cannot be devided by 6 without remainder)"));
		}
		else
			LWRITELN(LOG, _L("[Announce] No peers list / peers list invalid"));
	}	
	LWRITELN(LOG, _L("[Announce] Tracker response procesed"));
	
	HLWRITELN(LOG, _L("[Torrent] ProcessTrackerResponseL end"));
}

void CTracker::OnNetworkConnectionStartedL(TBool aResult, CNetworkConnection& /*aConnection*/, TInt /*aConnectionIndex*/)
{
	if (aResult && iIsAnnouncing)
	{
		AnnounceL(iCurrentEvent);
	}
}
	
void CTracker::OnNetworkConnectionDownL(CNetworkConnection& /*aConnection*/, TInt /*aConnectionIndex*/)
{
	HLWRITELN(LOG, _L("[CTracker] OnNetworkConnectionDownL begin"));
	
	delete iConnection;
	iConnection = NULL;
	
	HLWRITELN(LOG, _L("[CTracker] OnNetworkConnectionDownL end"));
}

void CTracker::NetworkManagerEventReceivedL(TKiNetworkManagerEvent /*aEvent*/)
{
	// no implementationrequired
}

