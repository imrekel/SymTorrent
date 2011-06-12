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

// INCLUDE FILES
#include "IAPSelectContainer.h"
#include "MobileAgent.rsg"
#include "Common.h"
#include "MobileAgentDocument.h"
#include "MobileAgentAppUi.h"
#include "MobileAgent.hrh"
#include <aknlists.h>
#include <mobileagent.mbg>
#include <AknIconArray.h>
#include <aknview.h>

namespace MobileAgent
{

// ================= MEMBER FUNCTIONS =======================


void CIAPSelectContainer::ConstructL(const TRect& aRect)
{
	iDocument = DOCUMENT;

    CreateWindowL();

    iListBox = new (ELeave) CAknSingleStyleListBox;
	iListBox->ConstructL(this, EAknListBoxSelectionList);
	iListBox->CreateScrollBarFrameL(ETrue);
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
	
    SetRect(aRect);
    ActivateL();
}


CIAPSelectContainer::~CIAPSelectContainer()
{
    delete iListBox;    
}

void CIAPSelectContainer::AppendItemL(const TDesC& aItemText)
{
	ItemArray()->AppendL(aItemText);
	iListBox->HandleItemAdditionL();
	//iListBox->
}



void CIAPSelectContainer::SizeChanged()
{    
    iListBox->SetRect(Rect());    
}


TInt CIAPSelectContainer::CurrentItemIndex()
{
	return iListBox->CurrentItemIndex();	
}


// ---------------------------------------------------------
// CIAPSelectContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CIAPSelectContainer::CountComponentControls() const
{
    return 1;
}


// ---------------------------------------------------------
// CIAPSelectContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CIAPSelectContainer::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
    {
        case 0:
            return iListBox;
        
        default:
            return NULL;
    }
}


TKeyResponse CIAPSelectContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
	if (aType == EEventKey) 
	{
		switch (aKeyEvent.iCode) 
		{
			case EKeyDevice3:
			{
				if (iListBox->CurrentItemIndex() >= 0)
				{
					APPUI->View(KIAPSelectViewId)->HandleCommandL(EAknSoftkeyOk);				
				}
				return EKeyWasConsumed;
			}
			
			case EKeyNo:
				return EKeyWasNotConsumed;	
			
		default:;			
		}
	}

	return iListBox->OfferKeyEventL(aKeyEvent, aType);
}

} // namespace MobileAgent

// End of File  
