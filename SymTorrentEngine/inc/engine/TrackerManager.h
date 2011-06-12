/*
 ============================================================================
 Name		: TrackerManager.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CTrackerManager declaration
 ============================================================================
 */

#ifndef TRACKERMANAGER_H
#define TRACKERMANAGER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "STEnums.h"
#include "Tracker.h"

// CLASS DECLARATION
class CSTTorrent;

/**
 * Interface for tracker manager event notifications
 */
class MTrackerManagerObserver
{
public:
	virtual void AnnouncedToAllTrackersL(TTrackerConnectionEvent aEvent) = 0;
};

/**
 *  CTrackerManager
 * 
 */
class CTrackerManager:  public CBase, 
						public MTrackerObserver
{
public:
	// Constructors and destructor
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CTrackerManager(CSTTorrent& aTorrent);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

	/**
	 * Destructor.
	 */
	~CTrackerManager();
	
public: // exported methods
	
	/**
	 * Creates an alphabetically sorted list of all addresses.
	 * The caller takes ownership of the list.
	 */
	IMPORT_C CDesC8Array* GetAllAddressesL() const;
	
	/**
	 * @return the number of trackers
	 */
	TInt TrackerCount() const { return iTrackers.Count(); }
	
public:
	
	/**
	 * Announces to the trackers.
	 * 
	 * If start or "notspecified" is given, the trackers will continue reconnecting periodically.
	 * If stop is used, trackers stop reconnecting after the announce is over.
	 */
	void AnnounceL(TTrackerConnectionEvent aEvent = ETrackerEventNotSpecified);
	
	/**
	 * @return the torrent whose trackers are managed
	 */
	CSTTorrent& Torrent() { return iTorrent; }
	
	/**
	 * @return the tracker manager observer
	 */
	void SetObserver(MTrackerManagerObserver* aObserver) { iObserver = aObserver; }

	/**
	 * Adds a new tracker to the managed trackers
	 */
	void AddTrackerL(const TDesC8& aAddress);
	
	/**
	 * The number of trackers that are active
	 */
	TInt GetAnnouncingTrackerCount();
	
private: // from MTrackerObserver
	
	void TrackerAnnounceFinishedL(CTracker& aTracker);

private:
	
	CSTTorrent& iTorrent;
	
	RPointerArray<CTracker> iTrackers;
	
	MTrackerManagerObserver* iObserver;
	
	/**
	 * The currently announced event
	 */
	TTrackerConnectionEvent iAnnouncedEvent;
};

#endif // TRACKERMANAGER_H
