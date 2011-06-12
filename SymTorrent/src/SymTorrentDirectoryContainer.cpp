/*****************************************************************************
 * Copyright (C) 2006,2007 Imre Kelényi
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
*  Name     : CSymTorrentDirectoryContainer from SymTorrentDirectoryContainer.h
*  Part of  : SymTorrent
*  Created  : 06.02.2007 by Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymTorrentDirectoryContainer.h"
#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin 
#include <akniconarray.h> 
#include <eikclbd.h>
#include <eikenv.h>
#include <aknlists.h>
#include "Symtorrent.hrh"
#include <SymTorrent.mbg>

#ifdef EKA2
	_LIT(KBitmapFile, "\\resource\\apps\\symtorrent.mif");
#else
	_LIT(KBitmapFile, "\\System\\Apps\\SymTorrent\\symtorrent.mbm");
#endif


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentDirectoryContainer::ConstructL
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymTorrentDirectoryContainer::ConstructL(const TRect& aRect, 
	CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr)
{
	iAppUi = aAppUi;
	iTorrentMgr = aTorrentMgr;
    CreateWindowL();

    iListBox = new (ELeave) CAknSingleGraphicStyleListBox();
    iListBox->SetMopParent(this);
	iListBox->ConstructL(this, EAknListBoxSelectionList);
	iListBox->SetContainerWindowL(*this);
	
	iListBox->View()->SetListEmptyTextL(_L("(empty)"));

	//create scrollbar
	iListBox->CreateScrollBarFrameL();
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

    SetRect(aRect);
   
    ActivateL();
}


CSymTorrentDirectoryContainer::~CSymTorrentDirectoryContainer()
{
    delete iListBox;
}


void CSymTorrentDirectoryContainer::SizeChanged()
{
    iListBox->SetRect(Rect());
}


TInt CSymTorrentDirectoryContainer::CountComponentControls() const
{
	return 1;
}


CCoeControl* CSymTorrentDirectoryContainer::ComponentControl(TInt aIndex) const
{
	switch ( aIndex )
		{
		case 0:
			return iListBox;
			
		default:
			return NULL;
		}
}


void CSymTorrentDirectoryContainer::Draw(const TRect& aRect) const
{
	CWindowGc& gc = SystemGc();
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
	AknsDrawUtils::Background( skin, cc, this, gc, aRect );
}


void CSymTorrentDirectoryContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}


TKeyResponse CSymTorrentDirectoryContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	 switch ( aKeyEvent.iCode )
	 {
		case EKeyUpArrow:
		{
			return iListBox->OfferKeyEventL(aKeyEvent,aType);
		}
		case EKeyDownArrow:				
		{
			return iListBox->OfferKeyEventL(aKeyEvent,aType);
		}
		case EKeyLeftArrow:				
		{
			iAppUi->ActivateMainViewL();
			break;
		}
		case EKeyRightArrow:				
		{
			iAppUi->ActivateSearchViewL();
			break;
		}
	    default:
		  return EKeyWasNotConsumed;		
	 }
	 return EKeyWasNotConsumed;
 
}

TTypeUid::Ptr CSymTorrentDirectoryContainer::MopSupplyObject(TTypeUid aId)
{
    return CCoeControl::MopSupplyObject( aId );
}


// End of File  

