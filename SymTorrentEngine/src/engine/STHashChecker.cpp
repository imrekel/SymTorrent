#include "STHashChecker.h"
#include "STTorrent.h"
#include "STPiece.h"
#include "STBitField.h"
#include <aknprogressdialog.h>
#include <eikprogi.h> 

CSTHashChecker::CSTHashChecker(CSTTorrent* aTorrent, CAknProgressDialog* aProgressDialog, MSTHashCheckerObserver* aHashCheckerObserver)
 : CActive(EPriorityStandard), iTorrent(aTorrent), iProgressDialog(aProgressDialog), iHashCheckerObserver(aHashCheckerObserver)
{	
}

CSTHashChecker::~CSTHashChecker()
{
	Cancel();
	delete iOriginalBitfield;
}

void CSTHashChecker::StartL()
{
	iPieceCount = iTorrent->PieceCount();
	iCurrentPieceIndex = 0;
	delete iOriginalBitfield;
	iOriginalBitfield = NULL;	
	iOriginalBitfield = iTorrent->BitField()->CloneL();
	
	iProgressDialog->GetProgressInfoL()->SetFinalValue(iPieceCount);
	iProgressDialog->SetCallback(this);
	iTorrent->ResetDownloadsL(EFalse);	
	
	SetActive();
	TRequestStatus*stat = &iStatus;
    User::RequestComplete(stat, KErrNone);
}

void CSTHashChecker::RunL()
{
	CSTPiece* piece = iTorrent->Piece(iCurrentPieceIndex);
	
	if (piece->CheckHashL())
	{
		iTorrent->PieceDownloadedL(piece, EFalse);
		iTorrent->UpdateBytesDownloadedL(piece->TotalSize(), EFalse);
		
		if (!piece->IsDownloaded())			
			piece->SetDownloaded(ETrue);	
	}
	else
	{
		iTorrent->iBitField->UnsetBit(piece->Index());
		
		if (piece->IsDownloaded())
			piece->SetDownloaded(EFalse);
	}
	
	iCurrentPieceIndex++;
	iProgressDialog->GetProgressInfoL()->IncrementAndDraw(1);
	
	if (iCurrentPieceIndex < iPieceCount)
	{
		TRequestStatus* stat = &iStatus;
    	User::RequestComplete(stat, KErrNone);
    	SetActive();
	}
	else
	{
		iProgressDialog->SetCallback(NULL);
		iProgressDialog->ProcessFinishedL();
		iHashCheckerObserver->HashCheckFinishedL(this);			
	}			
}

void CSTHashChecker::DoCancel()
{	
}

void CSTHashChecker::DialogDismissedL(TInt /*aButtonId*/)
{
	Cancel();
	
	iTorrent->ResetDownloadsL(EFalse);
	iTorrent->iBitField->SetL(iOriginalBitfield->Data());
	
	for (TInt i=0; i<iTorrent->PieceCount(); i++)
	{
		CSTPiece* piece = iTorrent->Piece(i);
		
		if (iOriginalBitfield->IsBitSet(i))
		{
			iTorrent->PieceDownloadedL(piece, EFalse);
			iTorrent->UpdateBytesDownloadedL(piece->TotalSize(), EFalse);
			
			if (!piece->IsDownloaded())			
				piece->SetDownloaded(ETrue);				
		}
		else
		{
			if (piece->IsDownloaded())
				piece->SetDownloaded(EFalse);			
		}
	}
	
	iHashCheckerObserver->HashCheckFinishedL(this);
}
