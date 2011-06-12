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
*  Name     : CIAPSelectContainer from IAPSelectContainer.h
*  Part of  : MobileAgent
*  Created  : 06.11.2005 by Imre Kelényi
* ============================================================================
*/

#ifndef IAPSELECTCONTAINER_H__
#define IAPSELECTCONTAINER_H__

// INCLUDES
#include <coecntrl.h>
#include <aknlists.h>
#include "STDefs.h"

// FORWARD DECLARATIONS
class CAknSingleStyleListBox;

namespace MobileAgent
{
	class CMobileAgentDocument;

	/**
	 *  CIAPSelectContainer 
	 */	
	class CIAPSelectContainer : public CCoeControl
	{
	public: 
    
		void ConstructL(const TRect& aRect);

		~CIAPSelectContainer();

		inline CAknSingleStyleListBox* ListBox() const;
		
		TInt CurrentItemIndex();
		
		void AppendItemL(const TDesC& aItemText);

	private:

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

		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

	private:

		inline CDesCArray* ItemArray();
    
	private:

		CAknSingleStyleListBox* iListBox;

		CMobileAgentDocument* iDocument;
	};


	inline CAknSingleStyleListBox* CIAPSelectContainer::ListBox() const {
		return iListBox;
	}

	inline CDesCArray* CIAPSelectContainer::ItemArray() {
		return static_cast<CDesCArray*>(iListBox->Model()->ItemTextArray());
	}

} // namespace MobileAgent


#endif

// End of File
