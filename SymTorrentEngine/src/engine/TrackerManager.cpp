/*
 ============================================================================
 Name		: TrackerManager.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CTrackerManager implementation
 ============================================================================
 */

#include "TrackerManager.h"
#include "STTorrent.h"
#include "Tracker.h"
#include "SymTorrentEngineLog.h"

CTrackerManager::CTrackerManager(CSTTorrent& aTorrent)
 : iTorrent(aTorrent)
{
	// No implementation required
}

CTrackerManager::~CTrackerManager()
{
	iTrackers.ResetAndDestroy();
}

void CTrackerManager::ConstructL()
{

}

void CTrackerManager::AddTrackerL(const TDesC8& aAddress)
{
	// throw away udp trackers for now
	_LIT8(KLitUdpPrefix, "udp://");
	if (aAddress.Length() >= KLitUdpPrefix().Length() && 
		aAddress.Left(KLitUdpPrefix().Length()).CompareF(KLitUdpPrefix) == 0)
	{
		return;
	}
	
	// check if the tracker has been already added
	for (TInt i=0; i<iTrackers.Count(); i++)
	{
		if (iTrackers[i]->Address() == aAddress)
			return;
	}
	
	LWRITE(LOG, _L("[CTrackerManager] Adding tracker: "));
	LWRITELN(LOG, aAddress);
	
	CTracker* tracker = new (ELeave) CTracker(*this);
	CleanupStack::PushL(tracker);
	tracker->ConstructL(aAddress);
	tracker->SetObserver(this);
	iTrackers.AppendL(tracker);
	CleanupStack::Pop(tracker);
}

void CTrackerManager::AnnounceL(TTrackerConnectionEvent aEvent)
{
	HLWRITELN(LOG, _L("[CTrackerManager] AnnounceL begin"));
	
	LWRITE(LOG, _L("[CTrackerManager] Announce initiated, event type: "));
	LWRITELN(LOG, aEvent);
	
	iAnnouncedEvent = aEvent;
	
	for (TInt i=0; i<iTrackers.Count(); i++)
	{
		// stop event is sent to only those tracker to which we have already announced ourselves
		if (aEvent != ETrackerEventStopped || iTrackers[i]->IsAnnounced())
		{
			iTrackers[i]->AnnounceL(aEvent);
			iTrackers[i]->SetObserver(this);
			
			// if the torrent is stopped and the network is not activated then stop connecting the trackers
			if (!Torrent().IsActive() &&
				!Torrent().TorrentMgr()->NetworkManager()->IsNetworkConnectionStarted(0))
				break;
		}
	}
	
	HLWRITELN(LOG, _L("[CTrackerManager] AnnounceL end"));
}

void CTrackerManager::TrackerAnnounceFinishedL(CTracker& aTracker)
{
	LWRITE(LOG, _L("[CTrackerManager] Tracker announche finished: "));
	LWRITELN(LOG, aTracker.Address());
	
	TBool allTrackerAnnounced = ETrue;
	for (TInt i=0; i<iTrackers.Count(); i++)
	{
		if (iTrackers[i]->IsAnnouncing())
		{
			allTrackerAnnounced = EFalse;
			break;
		}
	}
	
	if (allTrackerAnnounced)
	{
		for (TInt i=0; i<iTrackers.Count(); i++) // remove observer so we won't get notifications of automatic announces
			iTrackers[i]->SetObserver(NULL);
		
		if (iObserver)
			iObserver->AnnouncedToAllTrackersL(iAnnouncedEvent);
		
		iAnnouncedEvent = ETrackerEventNotSpecified;
	}
}

TInt CTrackerManager::GetAnnouncingTrackerCount()
{
	TInt count = 0;
	for (TInt i=0; i<iTrackers.Count(); i++)
	{
		if (iTrackers[i]->IsAnnouncing())
			count++;
	}
	
	return count;
}

EXPORT_C CDesC8Array* CTrackerManager::GetAllAddressesL() const
{
	CDesC8ArraySeg* addresses = new (ELeave) CDesC8ArraySeg(5);
	CleanupStack::PushL(addresses);
		
	for (TInt j=0; j<iTrackers.Count(); j++)
	{
		addresses->AppendL(iTrackers[j]->Address());
	}
	
	addresses->Sort();
	
	CleanupStack::Pop(); // addresses
	
	return addresses;
}
