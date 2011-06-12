/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
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
 * along with SymTorrent; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/*
* ============================================================================
*  Name     : CSymTorrentDetailsContainer from SymTorrentDetailsContainer.cpp
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#include "SymTorrentDetailsContainer.h"

#include <eiklabel.h>  // for example label control
#include <eikenv.h>
#include <aknutils.h>
#include <txtetext.h>
#include <txtrich.h>
#include <badesca.h>
#include "STTorrent.h"
#include "STStringFormatter.h"
#include "TrackerManager.h"
#include "TouchScrollableRichTextEditor.h"

#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin 

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymTorrentDetailsContainer::ConstructL(const TRect& aRect, CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr)
{
	iAppUi=aAppUi;
	iTorrentMgr=aTorrentMgr;
    CreateWindowL();

	iTorrent=iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());

    iTextWindow = 
		new (ELeave) CTouchScrollableRichTextEditor(TGulBorder(TGulBorder::ENone));

	iTextWindow->ConstructL(this, 0, 0,
		EEikEdwinNoAutoSelection|
		EEikEdwinDisplayOnly|
		EEikEdwinReadOnly|
		EEikEdwinAvkonDisableCursor|
		EEikEdwinNoHorizScrolling|
		EAknEditorNullInputMode);

	iTextWindow->CreateScrollBarFrameL()->
		SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto);	

	TBuf<256> buf;

	if (iTorrent->Name()!=KNullDesC)
	{
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Name:"));		
		iTextWindow->RichText()->AppendParagraphL();
		buf.Format(_L("%S"), &iTorrent->Name());
		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), buf);
		iTextWindow->RichText()->AppendParagraphL();
	}

	if (iTorrent->Path()!=KNullDesC)
	{
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Download path:"));
		iTextWindow->RichText()->AppendParagraphL();
		buf.Format(_L("%S"), &iTorrent->Path());
		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), buf);
		iTextWindow->RichText()->AppendParagraphL();
	}
	
	// Torrent Size
	SetTextBoldL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Torrent size:"));
	iTextWindow->RichText()->AppendParagraphL();
	buf.SetLength(0);
	TSTStringFormatter::AppendFileLength(iTorrent->Size(), buf);
	SetTextNormalL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), buf);
	iTextWindow->RichText()->AppendParagraphL();

	// Piece Size
	SetTextBoldL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Piece size:"));
	iTextWindow->RichText()->AppendParagraphL();
	buf.SetLength(0);
	TSTStringFormatter::AppendFileLength(iTorrent->PieceLength(), buf);
	SetTextNormalL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), buf);
	iTextWindow->RichText()->AppendParagraphL();

	if (iTorrent->TrackerManager().TrackerCount() != 0)
	{
		CDesC8Array* addresses = iTorrent->TrackerManager().GetAllAddressesL();
		CleanupStack::PushL(addresses);
		
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Announce URLs:"));
		
		for (TInt i=0; i<addresses->MdcaCount(); i++)
		{
			iTextWindow->RichText()->AppendParagraphL();
			
			TPtrC8 address = (*addresses)[i];

			HBufC* announce = HBufC::NewLC(address.Length());
			TPtr announcePtr(announce->Des());
			announcePtr.Copy(address);
			buf.Format(_L("%S"), &announcePtr);
			CleanupStack::PopAndDestroy(); // announce

			SetTextNormalL();
			iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), buf);
		}
		
		iTextWindow->RichText()->AppendParagraphL();
		
		CleanupStack::PopAndDestroy(); // addresses
	}
	
	/*if (iTorrent->TrackerRequestInterval() != 0)
	{
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Announce interval:"));
		iTextWindow->RichText()->AppendParagraphL();
		
		TInt interval = iTorrent->TrackerRequestInterval();
		TBuf<50> intervalBuf;
		
		if (iTorrent->TrackerRequestInterval() < 60)
		{
			intervalBuf.Num(interval);
			intervalBuf.Append(_L(" seconds"));
		}
		else
		{
			intervalBuf.Num(interval / 60);
			if ((interval / 60) == 1)
			{
				intervalBuf.Append(_L(" minute "));
			}
			else
				intervalBuf.Append(_L(" minutes "));
			
			if ((interval % 60) != 0)
			{
				intervalBuf.AppendNum(interval % 60);
				if ((interval % 60) == 1)
				{
					intervalBuf.Append(_L(" second"));
				}
				else
					intervalBuf.Append(_L(" seconds"));
			}
		}
		
		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), intervalBuf);
		iTextWindow->RichText()->AppendParagraphL();
	}*/
	
/*	if ((iTorrent->NextTrackerConnectionIn() > 0)  && (iTorrent->IsActive()))
	{
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Next announce in:"));
		iTextWindow->RichText()->AppendParagraphL();
		
		TInt interval = iTorrent->NextTrackerConnectionIn();
		TBuf<50> intervalBuf;
		
		if (iTorrent->TrackerRequestInterval() < 60)
		{
			intervalBuf.Num(interval);
			intervalBuf.Append(_L(" seconds"));
		}
		else
		{
			intervalBuf.Num(interval / 60);
			if ((interval / 60) == 1)
			{
				intervalBuf.Append(_L(" minute "));
			}
			else
				intervalBuf.Append(_L(" minutes "));
			
			if ((interval % 60) != 0)
			{
				intervalBuf.AppendNum(interval % 60);
				if ((interval % 60) == 1)
				{
					intervalBuf.Append(_L(" second"));
				}
				else
					intervalBuf.Append(_L(" seconds"));
			}
		}
		
		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), intervalBuf);
		iTextWindow->RichText()->AppendParagraphL();
	}*/
	
	if (iTorrent->LastTrackerConnectionTime().Int64() != 0)
	{				
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Last announce:"));
		iTextWindow->RichText()->AppendParagraphL();
		
		TDateTime time = iTorrent->LastTrackerConnectionTime().DateTime();
		TBuf<100> timeBuf;
		timeBuf.AppendNum(time.Hour());
		timeBuf.Append(_L(":"));
		timeBuf.AppendNum(time.Minute());
		
		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), timeBuf);
		iTextWindow->RichText()->AppendParagraphL();
	}

	if (iTorrent->Comment() != KNullDesC8)
	{
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Comment:"));
		iTextWindow->RichText()->AppendParagraphL();

		HBufC* tempData = HBufC::NewLC(iTorrent->Comment().Length());
		TPtr tempDataPtr(tempData->Des());
		tempDataPtr.Copy(iTorrent->Comment());	

		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), *tempData);
		iTextWindow->RichText()->AppendParagraphL();
		
		CleanupStack::PopAndDestroy(); // tempData
	}

	if (iTorrent->CreatedBy()!=KNullDesC8)
	{
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Created by:"));
		iTextWindow->RichText()->AppendParagraphL();

		TBuf<200> tempData;
		tempData.Copy(iTorrent->CreatedBy().Left(200));	

		buf.Format(_L("%S"), &tempData);

		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), buf);
	}
	
	if (iTorrent->IsFailed())
	{
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Failed:"));
		iTextWindow->RichText()->AppendParagraphL();
		
		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), iTorrent->FailReason());
	}
	
	iTextWindow->RichText()->AppendParagraphL();

	iTextWindow->SetFocus(ETrue);

    iBgContext = CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, TRect(0,0,1,1), EFalse );
    SetRect(aRect);
    ActivateL();
}

// Destructor
CSymTorrentDetailsContainer::~CSymTorrentDetailsContainer()
{
    delete iTextWindow;
	delete iBgContext;
}

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymTorrentDetailsContainer::SizeChanged()
{
	if ( iBgContext )
	{
		iBgContext->SetRect(Rect());
 
		if ( &Window() )
		{
			iBgContext->SetParentPos( PositionRelativeToScreen() );
		}
	}
	
	// set text window size
	TInt scrollbarWidth = iTextWindow->ScrollBarFrame()->VerticalScrollBar()->ScrollBarBreadth();
	TInt textWindowWidth = Rect().Width() - scrollbarWidth - 8;
	iTextWindow->SetExtent(TPoint(2, 5), TSize(textWindowWidth, Rect().Height()-5));
}

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymTorrentDetailsContainer::CountComponentControls() const
{
    return 1; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymTorrentDetailsContainer::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
        {
        case 0:
            return iTextWindow;
        default:
            return NULL;
        }
}

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSymTorrentDetailsContainer::Draw(const TRect& aRect) const
{
    CWindowGc& gc = SystemGc();

	// draw background
    // draw background
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	AknsDrawUtils::Background( skin, iBgContext, this, gc, aRect );
    	
	/*MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
    AknsDrawUtils::Background( skin, cc, this, gc, aRect );*/
}

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSymTorrentDetailsContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,
//	 TEventCode aType)
// ---------------------------------------------------------
//
TKeyResponse CSymTorrentDetailsContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	if (aType == EEventKey) 
	{
		switch (aKeyEvent.iCode) 
		{
			case EKeyUpArrow:
			{				
				iTextWindow->MoveCursorL(TCursorPosition::EFPageUp, EFalse);
				return EKeyWasConsumed;
			}

			case EKeyDownArrow:
			{				
				iTextWindow->MoveCursorL(TCursorPosition::EFPageDown, EFalse);
				return EKeyWasConsumed;
			}
			case EKeyNo:
				return EKeyWasNotConsumed;	
			default:;
		}
	}
	return EKeyWasNotConsumed;
}

// ---------------------------------------------------------
// CSymTorrentDetailsContainer::MopSupplyObject()
// Pass skin information if needed.
// ---------------------------------------------------------
//
TTypeUid::Ptr CSymTorrentDetailsContainer::MopSupplyObject(TTypeUid aId)
{
    if(aId.iUid == MAknsControlContext::ETypeId && iBgContext)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBgContext);
        }

    return CCoeControl::MopSupplyObject( aId );
}

void CSymTorrentDetailsContainer::SetTextNormalL()
{
	CCharFormatLayer* formatLayer = CEikonEnv::NewDefaultCharFormatLayerL();
	CleanupStack::PushL(formatLayer);
	TCharFormat charFormat; 
	TCharFormatMask charFormatMask;
	formatLayer->Sense(charFormat, charFormatMask);

	// Color
	charFormat.iFontPresentation.iTextColor = iAppUi->TextColor();
	charFormatMask.SetAttrib(EAttColor);
	
	// Font
	charFormat.iFontSpec.iTypeface = 
		LatinPlain12()->FontSpecInTwips().iTypeface;
	charFormatMask.SetAttrib(EAttFontTypeface);
	
	// Height
	charFormat.iFontSpec.iHeight = 120;
	charFormatMask.SetAttrib(EAttFontHeight);

	iTextWindow->RichText()->ApplyCharFormatL(
		charFormat, charFormatMask, iTextWindow->RichText()->DocumentLength(), 0);

	CleanupStack::PopAndDestroy(); // formatLayer

	CParaFormat* paraFormat = CParaFormat::NewLC();
	TParaFormatMask paraFormatMask;

	paraFormat->iLineSpacingControl = CParaFormat::ELineSpacingExactlyInTwips;
	paraFormatMask.SetAttrib(EAttLineSpacingControl);

	paraFormat->iLineSpacingInTwips = 135;
	paraFormatMask.SetAttrib(EAttLineSpacing);
	
	paraFormat->iSpaceAfterInTwips = 40;
	paraFormatMask.SetAttrib(EAttSpaceAfter);
	
	iTextWindow->RichText()->ApplyParaFormatL(paraFormat, paraFormatMask, iTextWindow->RichText()->DocumentLength(), 0);
	CleanupStack::PopAndDestroy(); // paraFormat	
}


void CSymTorrentDetailsContainer::SetTextBoldL()
{
	CCharFormatLayer* formatLayer = CEikonEnv::NewDefaultCharFormatLayerL();
	CleanupStack::PushL(formatLayer);
	TCharFormat charFormat; 
	TCharFormatMask charFormatMask; 
	formatLayer->Sense(charFormat, charFormatMask);

	// Color
	charFormat.iFontPresentation.iTextColor = iAppUi->TextColor();
	charFormatMask.SetAttrib(EAttColor);

	// Weight
	charFormat.iFontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
    charFormatMask.SetAttrib(EAttFontStrokeWeight);
    
    // Height
	charFormat.iFontSpec.iHeight = 120;
	charFormatMask.SetAttrib(EAttFontHeight);

	iTextWindow->RichText()->ApplyCharFormatL(charFormat, charFormatMask, iTextWindow->RichText()->DocumentLength(), 0);
	CleanupStack::PopAndDestroy(); // formatLayer

	CParaFormat* paraFormat = CParaFormat::NewLC();
	TParaFormatMask paraFormatMask;

	paraFormat->iLineSpacingControl = CParaFormat::ELineSpacingExactlyInTwips;
	paraFormatMask.SetAttrib(EAttLineSpacingControl);

	paraFormat->iLineSpacingInTwips = 135;
	paraFormatMask.SetAttrib(EAttLineSpacing);

	iTextWindow->RichText()->ApplyParaFormatL(paraFormat, paraFormatMask, iTextWindow->RichText()->DocumentLength(), 0);
	CleanupStack::PopAndDestroy(); // paraFormat
}

void CSymTorrentDetailsContainer::HandleResourceChange(TInt aType)
{
    CCoeControl::HandleResourceChange( aType );
 
    // application layout change request notification
    if ( aType == KEikDynamicLayoutVariantSwitch )
    {
        // apply new appropriate rect
        TRect rect;
        AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, rect );
        SetRect( rect );
    }
}

// End of File  
