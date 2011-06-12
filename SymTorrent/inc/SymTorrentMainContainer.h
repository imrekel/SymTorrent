/*****************************************************************************
 * Copyright (C) 2006 Imre Kelényi
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

/*
* ============================================================================
*  Name     : CSymTorrentMainContainer from SymTorrentMainContainer.h
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
* ============================================================================
*/

#ifndef SYMTORRENTMAINCONTAINER_H
#define SYMTORRENTMAINCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <aknlists.h> 
#include "SymTorrentAppUi.h"
#include "STTorrentManager.h"
#include "STDefs.h"
   
// FORWARD DECLARATIONS
class MAknsControlContext; // for skins support

// CLASS DECLARATION

/**
*  CSymTorrentMainContainer  container control class.
*  
*/
class CSymTorrentMainContainer : 	public CCoeControl, 
									public MCoeControlObserver, 
									public MSTTorrentObserver,
									public MEikListBoxObserver
{
public: // Constructors and destructor
    
    /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container, aAppUi SymTorrentAppUi, aTorrentMgr the TorrentManager.
    */
    void ConstructL(const TRect& aRect, CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr);

    /**
    * Destructor.
    */
    ~CSymTorrentMainContainer();

public: // New functions

   /**
	* Removes the current item
	*/
	void RemoveCurrentItemInList();
	
	inline CAknDoubleGraphicStyleListBox* ListBox();
	
public: // Functions from base classes

	
private:

	void FormatTorrentListItem(	TDes& aListItem,
								const TDesC& aName,
								TInt aConnectionCount,
								TReal aDownloadPercent,
								TReal aDownloadRate,
								TReal aUploadRate,
								TSTTorrentStatusInfo aStatus,
								TBool iIsActive,
								TBool aIsComplete,
								TBool aIsFailed);

private: // Functions from base classes
   /**
    * From CoeControl,SizeChanged.
    */
    void SizeChanged();

   /**
    * From CoeControl,CountComponentControls.
    */
    TInt CountComponentControls() const;

   /**
    * From CCoeControl,ComponentControl.
    */
    CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * From CCoeControl,Draw.
    */
    void Draw(const TRect& aRect) const;

	/**
	* Pass skin information if needed.
	*/
	TTypeUid::Ptr MopSupplyObject(TTypeUid aId);
   	
	/**
	* From MCoeControlObserver
	* Acts upon changes in the hosted control's state. 
	*
	* @param	aControl	The control changing its state
	* @param	aEventType	The type of control event 
	*/
    void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
    
	/**
	* Key events
	*/
	TKeyResponse OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType);
	
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
	
	void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType); 
	
public:

	void InsertTorrentL(CSTTorrent* aTorrent, TInt aIndex);
	
private:

	void RemoveTorrentL(TInt aIndex);
			
	void HandleCurrentItemClickedL();

private: // from MSTTorrentObserver

	virtual void TorrentChangedL(CSTTorrent* aTorrent, TInt aIndex, TUint32 aEventFlags);
	
private:
   
    CAknDoubleGraphicStyleListBox*			iTorrentListBox;
    
	CSymTorrentAppUi*						iAppUi;
	
	TInt									iInListIndex;
	
    CSTTorrentManager*						iTorrentMgr;
};

inline CAknDoubleGraphicStyleListBox* CSymTorrentMainContainer::ListBox() {
	return iTorrentListBox;
}

#endif

// End of File
