/*
 ============================================================================
 Name		: FailedPieceCollector.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFailedPieceCollector implementation
 ============================================================================
 */

#include "FailedPieceCollector.h"
#include "SymTorrentEngineLog.h"
#include "STPiece.h"

const TInt KToleratedFailureCount = 1;

CFailedPieceCollector::CFailedPieceCollector()
	{
	// No implementation required
	}

CFailedPieceCollector::~CFailedPieceCollector()
	{
	iFailedPieces.Reset();
	iDeniedPieces.Reset();
	}

CFailedPieceCollector* CFailedPieceCollector::NewLC()
	{
	CFailedPieceCollector* self = new (ELeave) CFailedPieceCollector();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CFailedPieceCollector* CFailedPieceCollector::NewL()
	{
	CFailedPieceCollector* self = CFailedPieceCollector::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

void CFailedPieceCollector::ConstructL()
	{

	}

void CFailedPieceCollector::RegisterFailedPieceL(CSTPiece& aPiece)
{
	TBool foundPiece = EFalse;
	
	for (TInt i=0; i<iFailedPieces.Count(); i++)
	{
		if (iFailedPieces[i].iPiece == &aPiece)
		{
			iFailedPieces[i].iFailureCount++;
			if (iFailedPieces[i].iFailureCount > KToleratedFailureCount) //move to denied pieces
			{
				iFailedPieces.Remove(i);
				iDeniedPieces.Append(&aPiece);
				
				LWRITE(LOG, _L("[CFailedPieceCollector] Piece ")); 
				LWRITE(LOG, aPiece.Index());
				LWRITELN(LOG, _L(" were added to denied pieces"));
			}
			
			break;
		}
	}
	
	// add to failed pieces
	if (!foundPiece)
	{
		for (TInt i=0; i<iDeniedPieces.Count(); i++)
		{
			if (iDeniedPieces[i] == &aPiece) // already denied?
				return;
		}
		
		TFailedPiece piece;
		piece.iPiece = &aPiece;
		piece.iFailureCount = 1;
		
		iFailedPieces.Append(piece);
	}
}

TBool CFailedPieceCollector::IsDenied(CSTPiece& aPiece)
{
	for (TInt i=0; i<iDeniedPieces.Count(); i++)
	{
		if (iDeniedPieces[i] == &aPiece)
			return ETrue;
	}
	
	return EFalse;
}
