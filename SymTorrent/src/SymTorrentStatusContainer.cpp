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
*  Name     : CSymTorrentStatusContainer from SymTorrentStatusContainer.cpp
*  Part of  : SymTorrent
*  Created  : 31.01.2006 by Imre Kelényi
*  Copyright: 2006
* ============================================================================
*/

// INCLUDE FILES
#include "SymTorrentStatusContainer.h"

#include <eiklabel.h>  // for example label control
#include <eikenv.h>
#include <aknutils.h>
#include <txtetext.h>
#include <txtrich.h>
#include "STTorrent.h"
#include "STStringFormatter.h"
#include "STTorrentManagerSingleton.h"
#include "STPreferences.h"
#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin 

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSymTorrentStatusContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CSymTorrentStatusContainer::ConstructL(const TRect& aRect, CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr)
{
	iAppUi = aAppUi;
	iTorrentMgr = aTorrentMgr;
    CreateWindowL();

    iTextWindow = 
		new (ELeave) CEikRichTextEditor(TGulBorder(TGulBorder::ENone));

	iTextWindow->ConstructL(this, 0, 0,
		EEikEdwinNoAutoSelection|
		EEikEdwinDisplayOnly|
		EEikEdwinReadOnly|
		EEikEdwinAvkonDisableCursor|
		EEikEdwinNoHorizScrolling|
		EAknEditorNullInputMode);

	iTextWindow->CreateScrollBarFrameL()->
		SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto);

	iTextWindow->SetFocus(ETrue);

    SetRect(aRect);
    iBackGround = CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, Rect(), EFalse );
    ActivateL();
}

// Destructor
CSymTorrentStatusContainer::~CSymTorrentStatusContainer()
{
    delete iTextWindow;
	delete iBackGround;
}

// ---------------------------------------------------------
// CSymTorrentStatusContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymTorrentStatusContainer::SizeChanged()
{
	TRect outputRect(5, 5, 
		Rect().Width() - 10, Rect().Height() - 5);
    iTextWindow->SetRect(outputRect);
}

// ---------------------------------------------------------
// CSymTorrentStatusContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymTorrentStatusContainer::CountComponentControls() const
{
    return 1; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CSymTorrentStatusContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymTorrentStatusContainer::ComponentControl(TInt aIndex) const
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
// CSymTorrentStatusContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSymTorrentStatusContainer::Draw(const TRect& aRect) const
{
    CWindowGc& gc = SystemGc();
    // TODO: Add your drawing code here
    // example code...
	// draw background
	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
    AknsDrawUtils::Background( skin, cc, this, gc, aRect );
}

// ---------------------------------------------------------
// CSymTorrentStatusContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSymTorrentStatusContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CSymTorrentStatusContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,
//	 TEventCode aType)
// ---------------------------------------------------------
//
TKeyResponse CSymTorrentStatusContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent, TEventCode aType)
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
			
			
			case EKeyLeftArrow:
			{
				iAppUi->ActivateMainViewL();
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
// CSymTorrentStatusContainer::MopSupplyObject()
// Pass skin information if needed.
// ---------------------------------------------------------
//
TTypeUid::Ptr CSymTorrentStatusContainer::MopSupplyObject(TTypeUid aId)
{
    if(aId.iUid == MAknsControlContext::ETypeId && iBackGround)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBackGround);
        }

    return CCoeControl::MopSupplyObject( aId );
}

void CSymTorrentStatusContainer::SetTextNormalL()
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
	
	//TInt fontHeight = LatinPlain12()->FontSpecInTwips().iHeight;
	//paraFormat->iLineSpacingInTwips = fontHeight * 2;
	//paraFormatMask.SetAttrib(EAttLineSpacing);

	//paraFormat->iSpaceBeforeInTwips = 0;
	//paraFormatMask.SetAttrib(EAttSpaceBefore);

	//paraFormat->iSpaceAfterInTwips = 0;
	//paraFormatMask.SetAttrib(EAttSpaceAfter);
	iTextWindow->RichText()->ApplyParaFormatL(paraFormat, paraFormatMask, iTextWindow->RichText()->DocumentLength(), 0);
	CleanupStack::PopAndDestroy(); // paraFormat	
}


void CSymTorrentStatusContainer::SetTextBoldL()
{
	CCharFormatLayer* formatLayer = CEikonEnv::NewDefaultCharFormatLayerL();
	CleanupStack::PushL(formatLayer);
	TCharFormat charFormat; 
	TCharFormatMask charFormatMask; 
	formatLayer->Sense(charFormat, charFormatMask);

	// Color
	charFormat.iFontPresentation.iTextColor = iAppUi->TextColor();
	charFormatMask.SetAttrib(EAttColor);
	
	// Height
	charFormat.iFontSpec.iHeight = 120;
	charFormatMask.SetAttrib(EAttFontHeight);

	// Weight
	charFormat.iFontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
    charFormatMask.SetAttrib(EAttFontStrokeWeight);

	iTextWindow->RichText()->ApplyCharFormatL(charFormat, charFormatMask, iTextWindow->RichText()->DocumentLength(), 0);
	CleanupStack::PopAndDestroy(); // formatLayer

	CParaFormat* paraFormat = CParaFormat::NewLC();
	TParaFormatMask paraFormatMask;

	paraFormat->iLineSpacingControl = CParaFormat::ELineSpacingExactlyInTwips;
	paraFormatMask.SetAttrib(EAttLineSpacingControl);

	//TInt fontHeight = LatinPlain12()->FontSpecInTwips().iHeight;
	paraFormat->iLineSpacingInTwips = 135;
	paraFormatMask.SetAttrib(EAttLineSpacing);

	//paraFormat->iSpaceBeforeInTwips = 0;
	//paraFormatMask.SetAttrib(EAttSpaceBefore);

	//paraFormat->iSpaceAfterInTwips = 0;
	//paraFormatMask.SetAttrib(EAttSpaceAfter);
	
	iTextWindow->RichText()->ApplyParaFormatL(paraFormat, paraFormatMask, iTextWindow->RichText()->DocumentLength(), 0);
	CleanupStack::PopAndDestroy(); // paraFormat	
}

void CSymTorrentStatusContainer::EngineStateChangedL(	TInetAddr /*aLocalAddress*/, 
														TInt aIncomingConnectionCount, 
														TInt aBytesDownloaded, 
														TInt aBytesUploaded)
{
	iTextWindow->RichText()->Reset();
	
	SetTextBoldL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Downloads in this session:"));
	iTextWindow->RichText()->AppendParagraphL();
	TBuf<100> downloadedBuf;
	TSTStringFormatter::AppendFileLength(aBytesDownloaded, downloadedBuf);
	SetTextNormalL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), downloadedBuf);
	
	iTextWindow->RichText()->AppendParagraphL();
	SetTextBoldL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Uploads in this session:"));
	iTextWindow->RichText()->AppendParagraphL();
	TBuf<100> uploadeddBuf;
	TSTStringFormatter::AppendFileLength(aBytesUploaded, uploadeddBuf);
	SetTextNormalL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), uploadeddBuf);
	iTextWindow->RichText()->AppendParagraphL();
	
	SetTextBoldL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Incoming connections:"));
	iTextWindow->RichText()->AppendParagraphL();
	TBuf<32> incomingConnBuf;
	incomingConnBuf.Num(aIncomingConnectionCount);
	SetTextNormalL();
	iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), incomingConnBuf);
	
	CSTTorrentManager* torrentMgr = TORRENTMGR;
	CNetworkManager* netMgr = torrentMgr->NetworkManager();
	
	if (netMgr->IsListening(0))
	{
		iTextWindow->RichText()->AppendParagraphL();
		
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Listening on:"));
		iTextWindow->RichText()->AppendParagraphL();
		
		TInetAddr localAddress;
		netMgr->GetLocalAddress(0, localAddress);
		
		TBuf<128> addressBuf;
		if (localAddress.Address() == 0)
		{
			addressBuf = _L("? (couldn't get local address)");
		}
		else
		{
			localAddress.Output(addressBuf);
			addressBuf.Append(_L(":"));
			TBuf<16> portBuf;
			portBuf.Num(localAddress.Port());
			addressBuf.Append(portBuf);
		}
				
		SetTextNormalL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), addressBuf);
	}
	
/*	if (PREFERENCES->IncomingConnectionsMode() == EEnabledWithProxy)
	{
		iTextWindow->RichText()->AppendParagraphL();
		SetTextBoldL();
		iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Proxy connection:"));
		iTextWindow->RichText()->AppendParagraphL();
		

		SetTextNormalL();
		
		switch (aProxyConnectorState)
		{	
			case EConConnecting:
			case EConInitializing:
			case EConConnected:
				iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("connecting"));
			break;
			
			case EConnectedAndInitialized:
				iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("connected"));
				
				if (aLocalAddress.Address() != 0)
				{
					iTextWindow->RichText()->AppendParagraphL();
					SetTextBoldL();
					iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("Address (through proxy):"));
					iTextWindow->RichText()->AppendParagraphL();
					
					SetTextNormalL();
					TBuf<64> addressBuf;
					aLocalAddress.Output(addressBuf);
					addressBuf.Append(_L(":"));
					
					TBuf<10> portBuf;
					portBuf.Num(aLocalAddress.Port());
					addressBuf.Append(portBuf);
					
					iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), addressBuf);
				}
				//iTextWindow->RichText()->AppendParagraphL();
			break;
			
			case EConOffline:			
			default:		
				iTextWindow->RichText()->InsertL(iTextWindow->RichText()->DocumentLength(), _L("offline"));
			break;
		}	
	}
	*/
	
	
	iTextWindow->HandleTextChangedL();
	//iTextWindow->SetCursorPosL(0, EFalse);
	
	iTextWindow->DrawDeferred();
}

// End of File  
