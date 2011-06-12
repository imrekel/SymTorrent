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
*  Name     : CSymTorrentFilesContainer from SymTorrentFilesContainer.h
*  Part of  : SymTorrent
*  Created  : 27.04.2006 by Péter Ekler
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#include "SymTorrentFilesContainer.h"

#ifdef EKA2
#include <AknsUtils.h>
#endif

#include <eiklabel.h>  // for example label control

#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin 
#include <akniconarray.h> 
#include <aknnotewrappers.h>
#include <eikclbd.h>
#include <aknlists.h>
#include <eikenv.h>
#include "Symtorrent.hrh"
#include "STTorrent.h"
#include "STFile.h"
#include "SymTorrentFilesView.h"
#include "SymTorrent.hrh"

#include <SymTorrent.mbg>

// Constants
#ifdef EKA2
	_LIT(KBitmapFile, "\\resource\\apps\\symtorrent.mif");
#else
	_LIT(KBitmapFile, "\\System\\Apps\\SymTorrent\\symtorrent.mbm");
	/*#ifdef __WINS__
	 	_LIT(KBitmapIconsC,
	 		"Z:\\system\\Apps\\SymTorrent\\SymTorrent.mbm");
	 	_LIT(KBitmapIconsE,
			"Z:\\system\\Apps\\SymTorrent\\SymTorrent.mbm");
	#else
	 	_LIT(KBitmapIconsC,
			"C:\\System\\Apps\\SymTorrent\\SymTorrent.mbm");
		_LIT(KBitmapIconsE,
			"E:\\System\\Apps\\SymTorrent\\SymTorrent.mbm");
	#endif*/
#endif


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentFilesContainer::ConstructL
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymTorrentFilesContainer::ConstructL(const TRect& aRect, 
	CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr)
{
	iAppUi=aAppUi;
	iTorrentMgr=aTorrentMgr;
	
	iTorrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());

    CreateWindowL();

    iFilesListBox = new (ELeave) CAknSingleGraphicStyleListBox();
    iFilesListBox->SetMopParent(this);
	iFilesListBox->ConstructL(this, EAknListBoxMultiselectionList);
	iFilesListBox->SetListBoxObserver(this);
	iFilesListBox->SetContainerWindowL(*this);
	iFilesListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
	iFilesListBox->ItemDrawer()->ColumnData()->SetMarqueeParams(5, 6, 3000000, 100000);

	// create scrollbar
	iFilesListBox->CreateScrollBarFrameL();
	iFilesListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
	
	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(2);
	// array of the icons
	CleanupStack::PushL(icons);
	
	#ifdef EKA2
	{
		CGulIcon* tempIcon1 = CGulIcon::NewL();
		CGulIcon* tempIcon2 = CGulIcon::NewL();
		CGulIcon* tempIcon3 = CGulIcon::NewL();
		CFbsBitmap* icon1 = NULL;
		CFbsBitmap* iconmask1 = NULL;
		CFbsBitmap* icon2 = NULL;
		CFbsBitmap* iconmask2 = NULL;
		CFbsBitmap* icon3 = NULL;
		CFbsBitmap* iconmask3 = NULL;
	
		AknIconUtils::CreateIconLC(icon1, iconmask1, KBitmapFile, 
			EMbmSymtorrentComplete, EMbmSymtorrentComplete_mask);
		AknIconUtils::CreateIconLC(icon2, iconmask2, KBitmapFile, 
			EMbmSymtorrentFailed, EMbmSymtorrentFailed_mask);
		AknIconUtils::CreateIconLC(icon3, iconmask3, KBitmapFile, 
			EMbmSymtorrentDownloading, EMbmSymtorrentDownloading_mask);
			
		tempIcon1->SetBitmap(icon1);
		tempIcon1->SetMask(iconmask1);

		tempIcon2->SetBitmap(icon2);
		tempIcon2->SetMask(iconmask2);
		
		tempIcon3->SetBitmap(icon3);
		tempIcon3->SetMask(iconmask3);
		
		CleanupStack::Pop(6); // icon1, iconmask1, icon2, iconmask2, icon3, iconmask3

		icons->AppendL(tempIcon1);				
		icons->AppendL(tempIcon2);
		icons->AppendL(tempIcon3);
	}	
	#else
	{
		CEikonEnv* eikEnv = CEikonEnv::Static();
		
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentComplete, EMbmSymtorrentComplete_mask));	
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentFailed, EMbmSymtorrentFailed_mask));
	}	
	#endif
			
	CleanupStack::Pop(icons); // icons
	iFilesListBox->ItemDrawer()->ColumnData()->SetIconArray(icons);
	
	iInListIndex=0;

    SetRect(aRect);
    iBackGround = CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, Rect(), EFalse );

    ActivateL();

	RebuildListL();
}

// Destructor
CSymTorrentFilesContainer::~CSymTorrentFilesContainer()
{
    delete iFilesListBox;
	delete iBackGround;
}

void CSymTorrentFilesContainer::RebuildListL()
{
	CDesCArray* itemArray = 
		new (ELeave) CDesCArrayFlat(3);
	CleanupStack::PushL(itemArray);
	
	_LIT(KLitFileFinished, "0\t");
	_LIT(KLitFileSkipped, "1\t");
	_LIT(KLitFileUnfinished, "2\t");	
	for (TInt i=0; i<iTorrent->FileCount(); i++)
	{
		const CSTFile* file = iTorrent->File(i);
		HBufC* item = HBufC::NewLC(file->FileName().Length() + 10);
		TPtr itemPtr(item->Des());
		
		TPtrC fileName(file->FileName());
		if (file->IsDownloaded())
			itemPtr.Append(KLitFileFinished);
		else
			if (file->IsSkipped())
				itemPtr.Append(KLitFileSkipped);
			else
				itemPtr.Append(KLitFileUnfinished);
		
		itemPtr.Append(fileName);
		itemPtr.Append(_L("\t\t"));
		
		itemArray->AppendL(*item);
	//	LOG->WriteLineL(*item);
		
		CleanupStack::PopAndDestroy(); // item				
	}
	
	CleanupStack::Pop(); // itemArray
	
	iFilesListBox->Model()->SetItemTextArray(itemArray);
	iFilesListBox->Model()->SetOwnershipType(ELbmOwnsItemArray);

	iFilesListBox->HandleItemAdditionL();
}

// ---------------------------------------------------------
// CSymTorrentFilesContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymTorrentFilesContainer::SizeChanged()
{
    iFilesListBox->SetRect(Rect());
}

// ---------------------------------------------------------
// CSymTorrentFilesContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymTorrentFilesContainer::CountComponentControls() const
{
	return 1;
}

// ---------------------------------------------------------
// CSymTorrentFilesContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymTorrentFilesContainer::ComponentControl(TInt aIndex) const
{
	switch ( aIndex )
		{
		case 0:
			return iFilesListBox;
		default:
			return NULL;
		}
}

// ---------------------------------------------------------
// CSymTorrentFilesContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSymTorrentFilesContainer::Draw(const TRect& aRect) const
{
	CWindowGc& gc = SystemGc();
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
	AknsDrawUtils::Background( skin, cc, this, gc, aRect );
}

// ---------------------------------------------------------
// CSymTorrentFilesContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSymTorrentFilesContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CSymTorrentFilesContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,
//	 TEventCode aType)
// ---------------------------------------------------------
//
TKeyResponse CSymTorrentFilesContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	 switch ( aKeyEvent.iCode )
	 {
		case EKeyUpArrow:
		{
			iFilesListBox->OfferKeyEventL(aKeyEvent,aType);
			return EKeyWasConsumed;
			break;
		}
		case EKeyDownArrow:				
		{
			iFilesListBox->OfferKeyEventL(aKeyEvent,aType);
			return EKeyWasConsumed;
			break;
		}
		case EKeyOK: 
		{
			TInt index = FilesListBox()->CurrentItemIndex();						
			if (index >= 0)
			{
				HandleCurrentItemClickedL();
            	return EKeyWasConsumed;
			}
			
			return EKeyWasNotConsumed;
			break;
		}
	    default:
		  return EKeyWasNotConsumed;		
	 }
	 return EKeyWasNotConsumed;
}

void CSymTorrentFilesContainer::HandleCurrentItemClickedL()
{
	TInt index = FilesListBox()->CurrentItemIndex();						
	if (index >= 0)
	{
		CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
    	const CSTFile* file = torrent->File(FilesListBox()->CurrentItemIndex());
    	
    	if (file->IsDownloaded())
    		static_cast<CSymTorrentAppUi*>(iAppUi)->OpenFileL(file->Path());
    	else
    		if (file->IsSkipped())
    			static_cast<CSymTorrentFilesView*>(iAppUi->View(TUid::Uid(ESymTorrentFilesView)))->
    				HandleCommandL(ESymTorrentCmdDownloadFile);
    		else
    			static_cast<CSymTorrentFilesView*>(iAppUi->View(TUid::Uid(ESymTorrentFilesView)))->
    			    HandleCommandL(ESymTorrentCmdDontDownloadFile);
	}
}

// ---------------------------------------------------------
// CSymTorrentFilesContainer::MopSupplyObject()
// Pass skin information if needed.
// ---------------------------------------------------------
//
TTypeUid::Ptr CSymTorrentFilesContainer::MopSupplyObject(TTypeUid aId)
{
    if(aId.iUid == MAknsControlContext::ETypeId && iBackGround)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBackGround);
        }

    return CCoeControl::MopSupplyObject( aId );
}


// ---------------------------------------------------------
// CSymTorrentFilesContainer::TorrentListBox()
// Returns the listBox;
// ---------------------------------------------------------
//
CAknSingleGraphicStyleListBox* CSymTorrentFilesContainer::FilesListBox()
{
	return iFilesListBox;
}

void CSymTorrentFilesContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
{
	if (aEventType == EEventItemDoubleClicked)
	{
		HandleCurrentItemClickedL();
	}
}

// End of File  
