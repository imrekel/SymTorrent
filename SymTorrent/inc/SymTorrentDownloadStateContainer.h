/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
 *----------------------------------------------------------------------------
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
*  Name     : CSymTorrentDownloadStateContainer from SymTorrentDownloadStateContainer.h
*  Part of  : SymTorrent
*  Created  : 06.03.2006 by Péter Ekler
*  Description:
*     Declares the downloadstate container control for application.
*  Version  :
*  Copyright: 2006
* ============================================================================
*/

#ifndef SYMTORRENTDOWNLOADSTATECONTAINER_H
#define SYMTORRENTDOWNLOADSTATECONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include "SymTorrentAppUi.h"
#include "STTorrentManager.h"
#include "STBitField.h"
#include "STDefs.h"
 
// FORWARD DECLARATIONS
class CEikLabel;        // for example labels
class MAknsControlContext; // for skins support

// CLASS DECLARATION

/**
*  CSymTorrentDownloadStateContainer  container control class.
*  
*/
class CSymTorrentDownloadStateContainer : public CCoeControl, public MCoeControlObserver, public MSTTorrentObserver
{
public: // Constructors and destructor

	CSymTorrentDownloadStateContainer();
    
    /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
    void ConstructL(const TRect& aRect, CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr);

    /**
    * Destructor.
    */
    ~CSymTorrentDownloadStateContainer();

private: // new

	void CalculatePixelPieceArray();
	
	void CalculatePieceColorArray();
	
	/**
	 * Sets the color of the label to the UI theme's default color
	 */
	void SetLabelColorL(CEikLabel& aLabel);

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
	
	/**
	 * Get notifications for layout changes
	 */
	void HandleResourceChange(TInt aType);
		
private: // from MSTTorrentObserver

	virtual void TorrentChangedL(CSTTorrent* aTorrent, TInt aIndex, TUint32 aEventFlags);

private: //data		
	MAknsControlContext*	iBackGround; // for skins support
	CEikLabel*				iNameLabel;
    CEikLabel*				iStatusLabel;
	CEikLabel*				iSizeLabel;
	CEikLabel*				iDownloadSpeedLabel;
	CEikLabel*				iUploadSpeedLabel;
	CEikLabel*				iConnectionCountLabel;
	TBool					iPercentArray[100];
	TInt					iPercent;
	TInt*					iPixelPieceArray;
	TInt*					iPieceColorArray;
	TInt					iPieceCount;
    CSTTorrentManager*		iTorrentMgr;
	CSymTorrentAppUi*		iAppUi;        
//	CSTTorrent*				iTorrent;
	const CSTBitField*		iBitField;
	const CSTBitField*		iToDownloadBitField;
	TBool					iDownloaded;
	TSize                	iApplicationWindowSize;
	
	TInt					iBarWidth;
	TInt					iMargin;
	TInt					iBarMargin;
};

#endif

// End of File
