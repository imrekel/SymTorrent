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
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/*
 * ============================================================================
 *  Name     : CSymTorrentDownloadStateContainer from SymTorrentDownloadStateContainer.cpp
 *  Part of  : SymTorrent
 *  Created  : 06.03.2006 by Péter Ekler
 * ============================================================================
 */

// INCLUDE FILES
#include "SymTorrentDownloadStateContainer.h"

#include <eiklabel.h>  // for example label control
#include <eikenv.h>
#include <aknutils.h>
#include <e32math.h>
#include <akntitle.h> 
#include "STTorrent.h"
#include "STStringFormatter.h"

#include <AknsDrawUtils.h>// skin
#include <AknsBasicBackgroundControlContext.h> //skin 

// Constants

_LIT(KDownloadStateStringFormat,
	 "Downloaded: %4.2f%%");
	 
_LIT(KDownloadStateEndGameStringFormat,
	 "Downloaded: %4.2f%% EG");

_LIT(KDownloadSpeedStringFormat,
	 "Download: %.2f kB/s");
	 
_LIT(KUploadSpeedStringFormat,
	 "Upload: %.2f kB/s");
	 
_LIT(KPeersStringFormat,
	 "Peers: %d(%d)");

_LIT(KTrackerConectingStringFormat, "Tracker connecting");

const TInt KBarPieceFullyDownloaded = 1000;
const TInt KBarPieceSkipped = 500;

// ================= MEMBER FUNCTIONS =======================

CSymTorrentDownloadStateContainer::CSymTorrentDownloadStateContainer()
 : iMargin(3), iBarMargin(7)
{
}

void CSymTorrentDownloadStateContainer::ConstructL(const TRect& aRect, CSymTorrentAppUi* aAppUi, CSTTorrentManager* aTorrentMgr)
{
	iAppUi = aAppUi;
	iTorrentMgr = aTorrentMgr;
    CreateWindowL();        

	iApplicationWindowSize = TSize(176, 208);
#ifdef EKA2
	AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EApplicationWindow, iApplicationWindowSize);
#endif

	CSTTorrent* torrent = iTorrentMgr->Torrent(iAppUi->SelectedTorrentIndex());
	
	aAppUi->TitlePane()->SetTextL(torrent->Name());

    iStatusLabel = new (ELeave) CEikLabel;
    iStatusLabel->SetContainerWindowL( *this );
    iStatusLabel->SetTextL( _L("Downloaded: 0%") );
    SetLabelColorL(*iStatusLabel);

	TBuf<100> buf;
	buf.Append(_L("Size: "));
	TSTStringFormatter::AppendFileLength(torrent->ToDownloadSize(), buf);

    iSizeLabel = new (ELeave) CEikLabel;
    iSizeLabel->SetContainerWindowL( *this );
    iSizeLabel->SetTextL(buf);
    SetLabelColorL(*iSizeLabel);

    iDownloadSpeedLabel = new (ELeave) CEikLabel;
    iDownloadSpeedLabel->SetContainerWindowL( *this );
    iDownloadSpeedLabel->SetTextL(_L("Download: 0 Kb/sec"));
    SetLabelColorL(*iDownloadSpeedLabel);
    
    iUploadSpeedLabel = new (ELeave) CEikLabel;
    iUploadSpeedLabel->SetContainerWindowL( *this );
    iUploadSpeedLabel->SetTextL(_L("Upload: 0 Kb/sec"));
    SetLabelColorL(*iUploadSpeedLabel);
    
    iConnectionCountLabel = new (ELeave) CEikLabel;
	iConnectionCountLabel->SetContainerWindowL( *this );
	iConnectionCountLabel->SetTextL(_L("Peers: 0(0)"));
	SetLabelColorL(*iConnectionCountLabel);
	
	iPieceCount = torrent->PieceCount();
	iBitField = torrent->BitField();
	iToDownloadBitField = torrent->ToDownloadBitField();
	iPercent = 0;
	iDownloaded = EFalse;
	
	CalculatePixelPieceArray();		
	
    SetRect(aRect);
    iBackGround = CAknsBasicBackgroundControlContext::NewL( KAknsIIDQsnBgAreaMain, Rect(), EFalse );

	TorrentChangedL(torrent, 0, 0);
	CalculatePieceColorArray();
	
	ActivateL();
}

void CSymTorrentDownloadStateContainer::CalculatePixelPieceArray()
{
	delete iPixelPieceArray;
	iPixelPieceArray = NULL;
	delete iPieceColorArray;
	iPieceColorArray = NULL;
	
	TInt tempSum = 0;
	TInt piece = 0;
	
	iPixelPieceArray = new (ELeave) TInt[iBarWidth];
	iPieceColorArray = new (ELeave) TInt[iBarWidth];
	for (TInt i=0; i<iBarWidth; i++)
		iPieceColorArray[i] = -1;

	if (iPieceCount <= iBarWidth)
	{		
		piece=0;
		for(TInt i=0; i<iBarWidth; i++)
		{
			tempSum += iPieceCount;
			if (tempSum > iBarWidth)
			{
				piece++;
				tempSum -= iBarWidth;
			}
			iPixelPieceArray[i] = piece;
		}
	}
	else
	{
		for(TInt i=0;i<iBarWidth;i++)
		{
			iPixelPieceArray[i] = ((i+1)*iPieceCount-tempSum) / iBarWidth;
			tempSum += iBarWidth * iPixelPieceArray[i];
		}
	}
}

// Destructor
CSymTorrentDownloadStateContainer::~CSymTorrentDownloadStateContainer()
{
	delete iConnectionCountLabel;
    delete iStatusLabel;
	delete iSizeLabel;
	delete iDownloadSpeedLabel;
	delete iUploadSpeedLabel;
	delete iBackGround;
	delete iPixelPieceArray;
	delete iPieceColorArray;
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateContainer::SizeChanged()
{
	AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EApplicationWindow, iApplicationWindowSize);

	iBarWidth = Rect().Width() - 2*iBarMargin;

	CalculatePixelPieceArray();
	CalculatePieceColorArray();
	
	TInt fontHeight = iSizeLabel->Font()->FontMaxHeight();
	TInt textWidth = iApplicationWindowSize.iWidth - 2*iMargin;

	iSizeLabel->SetExtent(   TPoint(iMargin, iMargin),
		TSize(textWidth, fontHeight) );
	iStatusLabel->SetExtent( TPoint(iMargin, iMargin + fontHeight),
		TSize(textWidth, fontHeight) );
	iDownloadSpeedLabel->SetExtent(  TPoint(iMargin, iMargin + 2*fontHeight),
		TSize(textWidth, fontHeight) );
	iUploadSpeedLabel->SetExtent(  TPoint(iMargin, iMargin + 3*fontHeight),
		TSize(textWidth, fontHeight) );		
	iConnectionCountLabel->SetExtent(  TPoint(iMargin, iMargin + 4*fontHeight),
		TSize(textWidth, fontHeight + 3) );
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CSymTorrentDownloadStateContainer::CountComponentControls() const
{
    return 5; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CSymTorrentDownloadStateContainer::ComponentControl(TInt aIndex) const
{
    switch ( aIndex )
        {
        case 0:
            return iStatusLabel;
        case 1:
            return iSizeLabel;
        case 2:
            return iDownloadSpeedLabel;
        case 3:
            return iUploadSpeedLabel;
        case 4:
            return iConnectionCountLabel;
        default:
            return NULL;
        }
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateContainer::Draw(const TRect& aRect) const
{
    CWindowGc& gc = SystemGc();

	MAknsSkinInstance* skin = AknsUtils::SkinInstance();
	MAknsControlContext* cc = AknsDrawUtils::ControlContext( this );
    AknsDrawUtils::Background( skin, cc, this, gc, aRect );
	
	TInt fontHeight = iSizeLabel->Font()->FontMaxHeight();

	TInt barRectTopLeftX = iBarMargin;
	TInt barRectTopLeftY = iBarMargin + 6*fontHeight + fontHeight/3;
	TInt barHeight = 2*fontHeight;
	
	TRect barRect(barRectTopLeftX-2, barRectTopLeftY-2,
		barRectTopLeftX+iBarWidth+2, barRectTopLeftY+barHeight+3);
		
	if (aRect.Intersects(barRect))
	{
		gc.SetPenColor(iAppUi->TextColor());
		gc.SetPenStyle(CGraphicsContext::ESolidPen);
		gc.SetPenSize(TSize(2,2));	

	/**	if (iDownloaded)
		{
			gc.SetBrushColor(TRgb(0, 255, 0));
			gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
		//	gc.DrawRect(TRect(barRectTopLeftX, barRectTopLeftY,
		//		barRectTopLeftX+iBarWidth, barRectTopLeftY+barHeight));
		}**/
		
		gc.DrawRect(barRect);
			
		//if (!iDownloaded)
		{
			gc.SetPenStyle(CGraphicsContext::ESolidPen);
			for (TInt i=0; i<iBarWidth; i++)
			{
				if (iPieceColorArray[i] >= 0)
				{
				//	TInt red = 0;
				//	TInt green = 255;
					
					if (iPieceColorArray[i] == KBarPieceFullyDownloaded)
						gc.SetPenColor(TRgb(0, 255, 0));
					else
					if (iPieceColorArray[i] == KBarPieceSkipped)
						gc.SetPenColor(TRgb(0, 0, 255));
					else
						gc.SetPenColor(TRgb(255-iPieceColorArray[i], 100 + (iPieceColorArray[i] / 2), 0));
					
					gc.DrawLine(TPoint(barRectTopLeftX+i, barRectTopLeftY),
						TPoint(barRectTopLeftX+i, barRectTopLeftY+barHeight));
				}
			}
		}
	}
		
	/*	if (iPieceCount<=iBarWidth)
		{
			gc.SetPenColor(TRgb(0,180,0));
			
			for(TInt i=0;i<iPieceCount;i++)
			{
				if (iBitField->IsBitSet(i))
				{
					for(TInt j=0;j<iBarWidth;j++)
					{
						if (iPixelPieceArray[j]==i)
						{
							gc.DrawLine(TPoint(barRectTopLeftX+j, barRectTopLeftY),
								TPoint(barRectTopLeftX+j, barRectTopLeftY+barHeight));
						}
					}
				}
			}
		}
		else
		{
			TInt currentBit=0, tempDepth, greenDepth;
			TBool allPieceInPixel;
			for(TInt i=0; i<iBarWidth; i++)
			{
				allPieceInPixel=ETrue;
				tempDepth=195/iPixelPieceArray[i];
				greenDepth=255;
				for(TInt j=0;j<iPixelPieceArray[i];j++)
				{
					if (iBitField->IsBitSet(currentBit+j))
					{
						gc.SetPenColor(TRgb(greenDepth,0,0));
						gc.SetPenStyle(CGraphicsContext::ESolidPen);
						gc.DrawLine(TPoint(barRectTopLeftX+i, barRectTopLeftY),
								TPoint(barRectTopLeftX+i, barRectTopLeftY+barHeight));
						greenDepth-=tempDepth;
					}
					allPieceInPixel=allPieceInPixel && iBitField->IsBitSet(currentBit+j);
				}
				if (allPieceInPixel)
				{
					gc.SetPenColor(TRgb(0,180,0));
					gc.SetPenStyle(CGraphicsContext::ESolidPen);
					gc.DrawLine(TPoint(barRectTopLeftX+i, barRectTopLeftY),
								TPoint(barRectTopLeftX+i, barRectTopLeftY+barHeight));
				}
				currentBit+=iPixelPieceArray[i];
			}
		}*/
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CSymTorrentDownloadStateContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
{
    // TODO: Add your control event handler code here
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,
//	 TEventCode aType)
// ---------------------------------------------------------
//
TKeyResponse CSymTorrentDownloadStateContainer::OfferKeyEventL(const TKeyEvent &aKeyEvent,TEventCode aType)
{
	if (aType == EEventKey) 
	{
		switch (aKeyEvent.iCode) 
		{
		case EKeyUpArrow:
		case EKeyDownArrow:
		default:;
		}
	}
	return EKeyWasNotConsumed;
}

// ---------------------------------------------------------
// CSymTorrentDownloadStateContainer::MopSupplyObject()
// Pass skin information if needed.
// ---------------------------------------------------------
//
TTypeUid::Ptr CSymTorrentDownloadStateContainer::MopSupplyObject(TTypeUid aId)
{
    if(aId.iUid == MAknsControlContext::ETypeId && iBackGround)
        {
        return MAknsControlContext::SupplyMopObject( aId, iBackGround);
        }

    return CCoeControl::MopSupplyObject( aId );
}

void CSymTorrentDownloadStateContainer::TorrentChangedL(CSTTorrent* aTorrent, TInt /*aIndex*/, TUint32 aEventFlags)
{
	if ((aEventFlags & ESTEventTorrentClosed) > 0)
	{

	}
	else
	{
		TInt connectionCount = aTorrent->ConnectionCount();
		TInt peerCount = aTorrent->PeerCount();
		TReal downloadPercent = aTorrent->DownloadPercent();
		TReal downloadRate = aTorrent->DownloadSpeed() / 1000;
		TReal uploadRate = aTorrent->UploadSpeed() / 1000;
		TBool isActive = aTorrent->IsActive();
		TBool isFailed = aTorrent->IsFailed();
		
		TBuf<100> buf;
		if (!aTorrent->IsComplete())
		{
			if (aTorrent->EndGame())
				buf.Format(KDownloadStateEndGameStringFormat, downloadPercent);
			else
				buf.Format(KDownloadStateStringFormat, downloadPercent);
			
			iStatusLabel->SetTextL(buf);
			iStatusLabel->DrawDeferred();
			
			if (isFailed)
				buf = _L("FAILED");
			else
			{
				if (isActive)
					buf.Format(KDownloadSpeedStringFormat, downloadRate);
				else
					buf = _L("PAUSED");
			}
			
			iDownloadSpeedLabel->SetTextL(buf);
			iDownloadSpeedLabel->MakeVisible(ETrue);
			iDownloadSpeedLabel->DrawDeferred();
		}
		else
		{
			iStatusLabel->SetTextL(_L("Download complete!"));
			iStatusLabel->DrawDeferred();
			iDownloaded = ETrue;
			
			if (isActive)
			{
				iDownloadSpeedLabel->MakeVisible(ETrue);
				iDownloadSpeedLabel->SetTextL(_L("SHARING"));
			}				
			else
				iDownloadSpeedLabel->MakeVisible(EFalse);
			
			iDownloadSpeedLabel->DrawDeferred();
		}				
		
		if (!isActive)
		{
			if (aTorrent->IsComplete())
				iConnectionCountLabel->MakeVisible(EFalse);
			else
				iConnectionCountLabel->MakeVisible(ETrue);
				
			iUploadSpeedLabel->MakeVisible(EFalse);
			iUploadSpeedLabel->DrawDeferred();
		}			
		else
		{
			iUploadSpeedLabel->MakeVisible(ETrue);
			iConnectionCountLabel->MakeVisible(ETrue);
			iConnectionCountLabel->DrawDeferred();
			
			buf.Format(KUploadSpeedStringFormat, uploadRate);
			iUploadSpeedLabel->SetTextL(buf);
			iUploadSpeedLabel->DrawDeferred();
		}
		
		// connections/peers label
		if (aTorrent->StatusInfo() == ETrackerConnecting)
			buf.Copy(KTrackerConectingStringFormat);
		else			
			buf.Format(KPeersStringFormat, connectionCount, peerCount);
		
		iConnectionCountLabel->SetTextL(buf);
		iConnectionCountLabel->DrawDeferred();
		
		iPercent = (TInt)downloadPercent;
		iBitField = aTorrent->BitField();
		
		if ((aEventFlags & ESTEventPieceDownloaded) > 0)
		{
			CalculatePieceColorArray();
			DrawDeferred();
		}		
	}		
}

void CSymTorrentDownloadStateContainer::CalculatePieceColorArray()
{
	for (TInt i=0; i<iBarWidth; i++)
		iPieceColorArray[i] = -1;
		
	if (iPieceCount <= iBarWidth)
	{
		for(TInt i=0; i<iPieceCount; i++)
		{						
			if (iBitField->IsBitSet(i))
			{
				for(TInt j=0; j<iBarWidth; j++)
				{
					if (iPixelPieceArray[j] == i)
						iPieceColorArray[j] = KBarPieceFullyDownloaded;
				}
			}
			else
				if (!iToDownloadBitField->IsBitSet(i))
				{
					for(TInt j=0; j<iBarWidth; j++)
					{
						if (iPixelPieceArray[j] == i)
							iPieceColorArray[j] = KBarPieceSkipped;
					}
				}
		}
	}
	else
	{
		TInt currentBit = 0;
		TInt tempDepth = 0;
		TInt greenDepth = 0;
		TBool allPieceInPixel = ETrue;
		
		// the color of the last pixel line which has at least one piece
		TInt lastValidPixelColor = -1;
		
		for (TInt i=0; i<iBarWidth; i++)
		{
			allPieceInPixel = ETrue;
			tempDepth = 255 / iPixelPieceArray[i];
			greenDepth = 255;
			
			if (iPixelPieceArray[i] == 0)
			{
				iPieceColorArray[i] = lastValidPixelColor;
				continue;
			}	
			
			TInt skippedPieceCount = 0;
			
			for (TInt j=0; j<iPixelPieceArray[i]; j++)
			{
				if (!iToDownloadBitField->IsBitSet(currentBit+j))
					skippedPieceCount++;
				
				if (iBitField->IsBitSet(currentBit+j))
				{
					iPieceColorArray[i] = greenDepth;
					greenDepth -= tempDepth;
				}
				allPieceInPixel = allPieceInPixel && iBitField->IsBitSet(currentBit+j);
			}
			
			if (allPieceInPixel)
			{
				iPieceColorArray[i] = KBarPieceFullyDownloaded;
			}
			else if (skippedPieceCount > 0 &&
					 skippedPieceCount >= iPixelPieceArray[i] / 2)
			{
				iPieceColorArray[i] = KBarPieceSkipped;
			}
			
			currentBit += iPixelPieceArray[i];
			lastValidPixelColor = iPieceColorArray[i];
		}
	}
}

void CSymTorrentDownloadStateContainer::SetLabelColorL(CEikLabel& aLabel)
{
	aLabel.OverrideColorL( EColorLabelTextEmphasis, iAppUi->TextColor() );
	aLabel.SetEmphasis( CEikLabel::EPartialEmphasis );    
	aLabel.SetBrushStyle( CWindowGc::ENullBrush );
}

void CSymTorrentDownloadStateContainer::HandleResourceChange(TInt aType)
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
