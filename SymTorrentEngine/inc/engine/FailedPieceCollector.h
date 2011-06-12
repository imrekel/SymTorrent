/*
 ============================================================================
 Name		: FailedPieceCollector.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFailedPieceCollector declaration
 ============================================================================
 */

#ifndef FAILEDPIECECOLLECTOR_H
#define FAILEDPIECECOLLECTOR_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

// CLASS DECLARATION
class CSTPiece;

struct TFailedPiece
	{
	CSTPiece* iPiece;
	TInt iFailureCount;
	};

/**
 *  CFailedPieceCollector
 *  
 *  Tracks pieces whose hash check was failed after downloading
 * 
 */
class CFailedPieceCollector : public CBase
{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CFailedPieceCollector();

	/**
	 * Two-phased constructor.
	 */
	static CFailedPieceCollector* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CFailedPieceCollector* NewLC();
	
	/**
	 * Number of failed pieces which has not been denied yet
	 */
	TInt FailedPieceCount() const { return iFailedPieces.Count(); }	
	/**
	 * Number of pieces which have been completly denied
	 */
	TInt DeniedPieceCount() const { return iDeniedPieces.Count(); }
	/**
	 * Registers a failed piece.
	 */
	void RegisterFailedPieceL(CSTPiece& aPiece);
	/**
	 * Checks if a piece is already failed too many times and thus cannot be downloaded
	 */
	TBool IsDenied(CSTPiece& aPiece);

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CFailedPieceCollector();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	
	/**
	 * Pieces which have at least once failed during hahs check
	 */
	RArray<TFailedPiece> iFailedPieces;
	
	/**
	 * Pieces which have failed to download to many times and thus are not allowed to download anymore
	 */
	RPointerArray<CSTPiece> iDeniedPieces;
};

#endif // FAILEDPIECECOLLECTOR_H
