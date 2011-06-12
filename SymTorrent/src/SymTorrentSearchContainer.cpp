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
*  Name     : CSymTorrentSearchContainer from SymTorrentSearchContainer.h
*  Part of  : SymTorrent
*  Created  : 06.02.2007 by Imre Kelényi
* ============================================================================
*/

// INCLUDE FILES
#include "SymTorrentSearchContainer.h"
#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin 
#include <akniconarray.h> 
#include <eikclbd.h>
#include <gulicon.h>
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
// CSymTorrentSearchContainer::ConstructL
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymTorrentSearchContainer::ConstructL(const TRect& aRect, 
	CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr)
{
	iAppUi=aAppUi;
	iTorrentMgr=aTorrentMgr;
    CreateWindowL();

    iListBox = new (ELeave) CAknDoubleGraphicStyleListBox();
    iListBox->SetMopParent(this);
	iListBox->ConstructL(this, EAknListBoxSelectionList);
	iListBox->SetContainerWindowL(*this);
	
	iListBox->View()->SetListEmptyTextL(_L("(empty)"));

	//create scrollbar
	iListBox->CreateScrollBarFrameL();
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
		CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
	
	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(1);  // must be initialized with the number of icons used by the list!
	CleanupStack::PushL(icons);

	#ifdef EKA2
	{
		CGulIcon* chainLinkIcon = CGulIcon::NewL();
		CleanupStack::PushL(chainLinkIcon);
		
		CFbsBitmap* chainLinkBmp = NULL;
		CFbsBitmap* chainLinkMask = NULL;
		AknIconUtils::CreateIconLC(chainLinkBmp, chainLinkMask, KBitmapFile,
			EMbmSymtorrentChainlink, EMbmSymtorrentChainlink_mask);	
		chainLinkIcon->SetBitmap(chainLinkBmp);
		chainLinkIcon->SetMask(chainLinkMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, chainLinkIcon);
		CleanupStack::Pop();				
	}
	#else
	{
		CEikonEnv* eikEnv = CEikonEnv::Static();
		
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentChainlink, EMbmSymtorrentChainlink_mask));	
	}
	#endif
			
	CleanupStack::Pop(icons); // icons
	iListBox->ItemDrawer()->ColumnData()->SetIconArray(icons);

    SetRect(aRect);
    
    CDesCArray* itemArray = static_cast<CDesCArray*>
		(iListBox->Model()->ItemTextArray());
	itemArray->InsertL(0, _L("0\tFirst line\tSecond line"));			
    
   
    ActivateL();
}


CSymTorrentSearchContainer::~CSymTorrentSearchContainer()
{
    delete iListBox;
}


void CSymTorrentSearchContainer::SizeChanged()
{
    iListBox->SetRect(Rect());
}


TInt CSymTorrentSearchContainer::CountComponentControls() const
{
	return 1;
}


CCoeControl* CSymTorrentSearchContainer::ComponentControl(TInt aIndex) const
{
	switch ( aIndex )
		{
		case 0:
			return iListBox;
			
		default:
			return NULL;
		}
}


void CSymTorrentSearchContainer::Draw(const TRect& aRect) const
{
	CWindowGc& gc = SystemGc();
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
	AknsDrawUtils::Background( skin, cc, this, gc, aRect );
}


void CSymTorrentSearchContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}


TKeyResponse CSymTorrentSearchContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
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
			iAppUi->ActivateDirectoryViewL();
			break;
		}
	    default:
		  return EKeyWasNotConsumed;		
	 }
	 return EKeyWasNotConsumed;
 
}

TTypeUid::Ptr CSymTorrentSearchContainer::MopSupplyObject(TTypeUid aId)
{
    return CCoeControl::MopSupplyObject( aId );
}


// End of File  

