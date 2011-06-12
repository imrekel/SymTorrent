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
*  Created  : 31.01.2006 by Péter Ekler, Imre Kelényi
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#include "SymTorrentMainContainer.h"

#include <eiklabel.h>  // for example label control

#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin 
#include <akniconarray.h> 
#include <aknnotewrappers.h>
#include <eikclbd.h>
#include <eikenv.h>
#include <aknlists.h>
#include "Symtorrent.hrh"
#include "STTorrent.h"
#include <SymTorrent.mbg>
#include "SymTorrentLog.h"

#ifdef EKA2
	_LIT(KBitmapFile, "\\resource\\apps\\symtorrent.mif");
#else
	_LIT(KBitmapFile, "\\System\\Apps\\SymTorrent\\symtorrent.mbm");
/*	#ifdef __WINS__
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

_LIT(KTorrentListFormat,
	 "0\t%S\t%4.2f%% Con: %d, %4.2fkB/s");

_LIT(KTorrentListFormatComplete,
	 "1\t%S\t100%% Con: %d");

_LIT(KTorrentListFormatCompleteSharing,
	 "1\t%S\t100%% Con: %d, SHARING");

_LIT(KTorrentListFormatTrackerFailed,
	 "0\t%S\t[Tracker] failed");

_LIT(KTorrentListFormatTrackerSuccessful,
	 "0\t%S\t[Tracker] connected");

_LIT(KTorrentListFormatTrackerConnecting,
	 "0\t%S\t[Tracker] connecting");

_LIT(KTorrentListFormatTorrentFailed,
	 "3\t%S\t%4.2f%% FAILED");

_LIT(KTorrentListFormatPaused,
	 "2\t%S\t%4.2f%% PAUSED");
	 
_LIT(KTorrentListFormatClosing,
	 "2\t%S\t%4.2f%% CLOSING");


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentMainContainer::ConstructL
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymTorrentMainContainer::ConstructL(const TRect& aRect, 
	CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr)
{
	iAppUi=aAppUi;
	iTorrentMgr=aTorrentMgr;
    CreateWindowL();

    iTorrentListBox = new (ELeave) CAknDoubleGraphicStyleListBox();
    iTorrentListBox->SetMopParent(this);
	iTorrentListBox->ConstructL(this,EAknListBoxSelectionList);
	iTorrentListBox->SetListBoxObserver(this);
	iTorrentListBox->SetContainerWindowL(*this);
	iTorrentListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
	iTorrentListBox->ItemDrawer()->ColumnData()->SetMarqueeParams(3, 6, 3000000, 100000);
	
	iTorrentListBox->View()->SetListEmptyTextL(_L("Tap here or select Options to add a torrent"));

	//create scrollbar
	iTorrentListBox->CreateScrollBarFrameL();
	iTorrentListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
	CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
	
	CArrayPtr<CGulIcon>* icons = new (ELeave) CAknIconArray(7);
	CleanupStack::PushL(icons);

	#ifdef EKA2
	{
		CGulIcon* downloadingIcon = CGulIcon::NewL();
		CleanupStack::PushL(downloadingIcon);
		CGulIcon* completeIcon = CGulIcon::NewL();
		CleanupStack::PushL(completeIcon);
		CGulIcon* pausedIcon = CGulIcon::NewL();
		CleanupStack::PushL(pausedIcon);
		CGulIcon* failedIcon = CGulIcon::NewL();
		CleanupStack::PushL(failedIcon);
		CGulIcon* uploadingIcon = CGulIcon::NewL();
		CleanupStack::PushL(uploadingIcon);
		CGulIcon* notUploadingIcon = CGulIcon::NewL();
		CleanupStack::PushL(notUploadingIcon);
		CGulIcon* notSharingIcon = CGulIcon::NewL();
		CleanupStack::PushL(notSharingIcon);
		
		CFbsBitmap* notSharingBmp = NULL;
		CFbsBitmap* notSharingMask = NULL;
		AknIconUtils::CreateIconLC(notSharingBmp, notSharingMask, KBitmapFile,
			EMbmSymtorrentNot_sharing, EMbmSymtorrentNot_sharing_mask);	
		notSharingIcon->SetBitmap(notSharingBmp);
		notSharingIcon->SetMask(notSharingMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, notSharingIcon);
		CleanupStack::Pop();
		
		CFbsBitmap* notUploadingBmp = NULL;
		CFbsBitmap* notUploadingMask = NULL;
		AknIconUtils::CreateIconLC(notUploadingBmp, notUploadingMask, KBitmapFile,
			EMbmSymtorrentNot_uploading, EMbmSymtorrentNot_uploading_mask);		
		notUploadingIcon->SetBitmap(notUploadingBmp);
		notUploadingIcon->SetMask(notUploadingMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, notUploadingIcon);
		CleanupStack::Pop();
		
		CFbsBitmap* uploadingBmp = NULL;
		CFbsBitmap* uploadingMask = NULL;
		AknIconUtils::CreateIconLC(uploadingBmp, uploadingMask, KBitmapFile,
			EMbmSymtorrentUploading, EMbmSymtorrentUploading_mask);		
		uploadingIcon->SetBitmap(uploadingBmp);
		uploadingIcon->SetMask(uploadingMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, uploadingIcon);
		CleanupStack::Pop();
		
		CFbsBitmap* failedBmp = NULL;
		CFbsBitmap* failedMask = NULL;
		AknIconUtils::CreateIconLC(failedBmp, failedMask, KBitmapFile,
			EMbmSymtorrentFailed, EMbmSymtorrentFailed_mask);		
		failedIcon->SetBitmap(failedBmp);
		failedIcon->SetMask(failedMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, failedIcon);
		CleanupStack::Pop();
		
		CFbsBitmap* pausedBmp = NULL;
		CFbsBitmap* pausedMask = NULL;
		AknIconUtils::CreateIconLC(pausedBmp, pausedMask, KBitmapFile,
			EMbmSymtorrentPaused, EMbmSymtorrentPaused_mask);		
		pausedIcon->SetBitmap(pausedBmp);
		pausedIcon->SetMask(pausedMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, pausedIcon);
		CleanupStack::Pop();
			
		CFbsBitmap* completeBmp = NULL;
		CFbsBitmap* completeMask = NULL;
		AknIconUtils::CreateIconLC(completeBmp, completeMask, KBitmapFile,
			EMbmSymtorrentComplete, EMbmSymtorrentComplete_mask);		
		completeIcon->SetBitmap(completeBmp);
		completeIcon->SetMask(completeMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, completeIcon);
		CleanupStack::Pop();	
		
		CFbsBitmap* downloadingBmp = NULL;
		CFbsBitmap* downloadingMask = NULL;
		AknIconUtils::CreateIconLC(downloadingBmp, downloadingMask, KBitmapFile,
			EMbmSymtorrentDownloading, EMbmSymtorrentDownloading_mask);		
		downloadingIcon->SetBitmap(downloadingBmp);
		downloadingIcon->SetMask(downloadingMask);
		CleanupStack::Pop(2);
		icons->InsertL(0, downloadingIcon);
		CleanupStack::Pop();
	}
	#else
	{
		CEikonEnv* eikEnv = CEikonEnv::Static();
		
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentDownloading, EMbmSymtorrentDownloading_mask));
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentComplete, EMbmSymtorrentComplete_mask));
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentPaused, EMbmSymtorrentPaused_mask));
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentFailed, EMbmSymtorrentFailed_mask));
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentUploading, EMbmSymtorrentUploading_mask));
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentNot_uploading, EMbmSymtorrentNot_uploading_mask));
		icons->AppendL(eikEnv->CreateIconL(KBitmapFile, 
			EMbmSymtorrentNot_sharing, EMbmSymtorrentNot_sharing_mask));
	}
	#endif
		
/*	CGulIcon* tempIcon1 = CGulIcon::NewL();
	CGulIcon* tempIcon2 = CGulIcon::NewL();
	CFbsBitmap* icon1 = NULL;
	CFbsBitmap* iconmask1 = NULL;
	CFbsBitmap* icon2 = NULL;
	CFbsBitmap* iconmask2 = NULL;
	
	icon1 = new (ELeave) CFbsBitmap();
	iconmask1 = new (ELeave) CFbsBitmap();
	icon2 = new (ELeave) CFbsBitmap();
	iconmask2 = new (ELeave) CFbsBitmap();

	if (icon1->Load(KBitmapIconsC, EIcon1) != KErrNone)
		icon1->Load(KBitmapIconsE, EIcon1);

	if (iconmask1->Load(KBitmapIconsC, EIcon1_mask) != KErrNone)
		iconmask1->Load(KBitmapIconsE, EIcon1_mask);		

	if (icon2->Load(KBitmapIconsC, EIcon2) != KErrNone)
		icon2->Load(KBitmapIconsE, EIcon2);

	if (iconmask2->Load(KBitmapIconsC, EIcon2_mask) != KErrNone)
		iconmask2->Load(KBitmapIconsE, EIcon2_mask);
	
	tempIcon1->SetBitmap(icon1);
	tempIcon1->SetMask(iconmask1);
	icons->AppendL(tempIcon1);	
		
	tempIcon2->SetBitmap(icon2);
	tempIcon2->SetMask(iconmask2);
	icons->AppendL(tempIcon2);	*/
	
	CleanupStack::Pop(icons); // icons
	iTorrentListBox->ItemDrawer()->ColumnData()->SetIconArray(icons);

	iInListIndex = 0;

    SetRect(aRect);
   
    ActivateL();
}

// Destructor
CSymTorrentMainContainer::~CSymTorrentMainContainer()
{
    delete iTorrentListBox;
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymTorrentMainContainer::SizeChanged()
{
    iTorrentListBox->SetRect(Rect());
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymTorrentMainContainer::CountComponentControls() const
{
	return 1;
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymTorrentMainContainer::ComponentControl(TInt aIndex) const
{
	switch ( aIndex )
		{
		case 0:
			return iTorrentListBox;
			
		default:
			return NULL;
		}
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSymTorrentMainContainer::Draw(const TRect& aRect) const
{
	CWindowGc& gc = SystemGc();
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
	AknsDrawUtils::Background( skin, cc, this, gc, aRect );
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSymTorrentMainContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,
//	 TEventCode aType)
// ---------------------------------------------------------
//
TKeyResponse CSymTorrentMainContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	 switch ( aKeyEvent.iCode )
	 {
		case EKeyUpArrow:
		{
			return iTorrentListBox->OfferKeyEventL(aKeyEvent,aType);
		}
		case EKeyDownArrow:				
		{
			return iTorrentListBox->OfferKeyEventL(aKeyEvent,aType);
		}
		case EKeyRightArrow:				
		{
			iAppUi->ActivateStatusViewL();
			break;
		}
		case EKeyOK: 
		{
			if (iTorrentListBox->CurrentItemIndex() >= 0)
			{
				HandleCurrentItemClickedL();				
				return EKeyWasConsumed;
			}
			break;
		}
		case EKeyBackspace:
		{
			TInt currentItemIndex = iTorrentListBox->CurrentItemIndex();
			
			if (currentItemIndex >= 0)
			{
				if (iEikonEnv->QueryWinL(_L("Remove torrent?"), _L("")))
					iTorrentMgr->RemoveTorrentL(currentItemIndex);		
				return EKeyWasConsumed;
			}			
			break;
		}
	    default:
		  return EKeyWasNotConsumed;		
	 }
	 return EKeyWasNotConsumed;
 
}

void CSymTorrentMainContainer::HandleCurrentItemClickedL()
{
	if (iTorrentListBox->CurrentItemIndex() >= 0)
	{
		iAppUi->SetSelectedTorrentIndex(iTorrentListBox->CurrentItemIndex());
		iAppUi->ActivateDownloadStateViewL();
	}
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::MopSupplyObject()
// Pass skin information if needed.
// ---------------------------------------------------------
//
TTypeUid::Ptr CSymTorrentMainContainer::MopSupplyObject(TTypeUid aId)
{
   /* if(aId.iUid == MAknsControlContext::ETypeId && iBackGround)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBackGround);
        }*/

    return CCoeControl::MopSupplyObject( aId );
}

// ---------------------------------------------------------
// CSymTorrentMainContainer::RemoveCurrentItemInList(TBool aWelcomeScreenState)
// Removes the current item - DEPRECATED
// ---------------------------------------------------------
//
void CSymTorrentMainContainer::RemoveCurrentItemInList()
{
	CAknWarningNote* note = new(ELeave) CAknWarningNote;
	note->ExecuteLD(_L("Under construction!"));
}


void CSymTorrentMainContainer::TorrentChangedL(CSTTorrent* aTorrent, TInt aIndex, TUint32 aEventFlags)
{
	HLWRITELN(LOG, _L("[CSymTorrentMainContainer::TorrentChangedL] begin"));
	
	if (aIndex >= 0)
	{
		if ((aEventFlags & ESTEventTorrentClosed) > 0)
		{
			RemoveTorrentL(aIndex);
		}
		else
			if ((aEventFlags & ESTEventTorrentOpened) > 0)
			{
				InsertTorrentL(aTorrent, aIndex);
			}
			else
			{
				const TDesC& name = aTorrent->Name();
				TInt connectionCount = aTorrent->ConnectionCount();
				TReal downloadPercent = aTorrent->DownloadPercent();
				TReal downloadRate = aTorrent->DownloadSpeed() / 1000;
				TReal uploadRate = aTorrent->UploadSpeed() / 1000;
				TSTTorrentStatusInfo status = aTorrent->StatusInfo();
				TBool isActive = aTorrent->IsActive();
				TBool isComplete = aTorrent->IsComplete();
				TBool isFailed = aTorrent->IsFailed();
				
				TBuf<256> insertString;		
				TInt current = iTorrentListBox->CurrentItemIndex();
				
				CDesCArray* itemArray = static_cast<CDesCArray*>
					(iTorrentListBox->Model()->ItemTextArray());

				itemArray->Delete(aIndex);

				FormatTorrentListItem(insertString, name, connectionCount, 
					downloadPercent, downloadRate, uploadRate, status, isActive, isComplete, isFailed);
				itemArray->InsertL(aIndex, insertString);
				iTorrentListBox->HandleItemAdditionL();

				iTorrentListBox->SetCurrentItemIndex(current);

				DrawDeferred();
			}		
	}
	HLWRITELN(LOG, _L("[CSymTorrentMainContainer::TorrentChangedL] end"));
}

void CSymTorrentMainContainer::InsertTorrentL(CSTTorrent* aTorrent, TInt aIndex)
{
	HLWRITELN(LOG, _L("[CSymTorrentMainContainer::InsertTorrentL] begin"));
	
	TBuf<256> insertString;		
	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iTorrentListBox->Model()->ItemTextArray());
		
	const TDesC& name = aTorrent->Name();
	TInt connectionCount = aTorrent->ConnectionCount();
	TReal downloadPercent = aTorrent->DownloadPercent();
	TReal downloadRate = aTorrent->DownloadSpeed();
	TReal uploadRate = aTorrent->UploadSpeed();
	TSTTorrentStatusInfo status = aTorrent->StatusInfo();
	TBool isActive = aTorrent->IsActive();
	TBool isComplete = aTorrent->IsComplete();
	TBool isFailed = aTorrent->IsFailed();
	
	FormatTorrentListItem(insertString, name, connectionCount,
		downloadPercent, downloadRate, uploadRate, status, isActive, isComplete, isFailed);

	itemArray->InsertL(aIndex,insertString);
	iTorrentListBox->HandleItemAdditionL();
	iTorrentListBox->SetCurrentItemIndexAndDraw(itemArray->Count() - 1);
	
	HLWRITELN(LOG, _L("[CSymTorrentMainContainer::InsertTorrentL] end"));
}

void CSymTorrentMainContainer::RemoveTorrentL(TInt aIndex)
{
	TInt current=iTorrentListBox->CurrentItemIndex();

	CDesCArray* itemArray = static_cast<CDesCArray*>
		(iTorrentListBox->Model()->ItemTextArray());

	itemArray->Delete(aIndex);

	AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(iTorrentListBox, aIndex, (aIndex == current));
	
	iTorrentListBox->HandleItemAdditionL();
	DrawDeferred();
}

void CSymTorrentMainContainer::FormatTorrentListItem(	TDes& aListItem,
														const TDesC& aName,
														TInt aConnectionCount,													
														TReal aDownloadPercent,
														TReal aDownloadRate,
														TReal aUploadRate,
														TSTTorrentStatusInfo aStatus,
														TBool aIsActive,
														TBool aIsComplete,
														TBool aIsFailed)
{
	HLWRITELN(LOG, _L("[CSymTorrentMainContainer::FormatTorrentListItem] begin"));
	
	if (aStatus == ETorrentRemoving)
	{
		aListItem.Format(KTorrentListFormatClosing, &aName, aDownloadPercent);		
	}
	else
	{
		if (aIsComplete)
		{
			if (aIsActive)
				aListItem.Format(KTorrentListFormatCompleteSharing, &aName, aConnectionCount);
			else
				aListItem.Format(KTorrentListFormatComplete, &aName, aConnectionCount);
		}		
		else
		{
			if (!aIsActive)
			{
				if (aIsFailed)
					aListItem.Format(KTorrentListFormatTorrentFailed, &aName, aDownloadPercent);
				else
					aListItem.Format(KTorrentListFormatPaused, &aName, aDownloadPercent);
			}
			else
				
				switch (aStatus)
				{
					case ETrackerConnecting:
						aListItem.Format(KTorrentListFormatTrackerConnecting, &aName);
						break;
					case ETrackerFailed:
						aListItem.Format(KTorrentListFormatTrackerFailed, &aName);
						break;
					case ETrackerSuccessful:
						aListItem.Format(KTorrentListFormatTrackerSuccessful, &aName);
						break;					
					default:
						aListItem.Format(KTorrentListFormat, &aName, aDownloadPercent, aConnectionCount, aDownloadRate);
						break;
				}
		}
	}
	
	
	if (aIsActive)
	{
		if ((aConnectionCount > 0) && (aUploadRate > 0))
			aListItem.Append(_L("\t4"));
		else
			aListItem.Append(_L("\t5"));
	}
	else
		aListItem.Append(_L("\t6"));
	
	HLWRITELN(LOG, _L("[CSymTorrentMainContainer::FormatTorrentListItem] end"));
}

void CSymTorrentMainContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
{
	if (aEventType == EEventItemDoubleClicked)
	{
		HandleCurrentItemClickedL();
	}
}

void CSymTorrentMainContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	if (aPointerEvent.iType == TPointerEvent::EButton1Down &&
		iTorrentMgr->TorrentCount() == 0)
	{
		iAppUi->HandleCommandL(ESymTorrentCmdChooseTorrentFile);
	}
	else
		CCoeControl::HandlePointerEventL(aPointerEvent);
}


// End of File  
