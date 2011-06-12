/*
 ============================================================================
 Name		: STPieceAccess.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CSTPieceAccess declaration
 ============================================================================
 */

#ifndef STPIECEACCESS_H
#define STPIECEACCESS_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include "STFileManager.h"

// FORWARD DECLARATIONS
class CSTPiece;
class CSTPeer;

// CLASS DECLARATION
class MPieceWriteObserver
{
public:
	virtual void OnPieceWriteFailedL(const CSTPiece& aPiece, TInt aBegin, TInt aLength) = 0;
	virtual void OnPieceWriteCompleteL(const CSTPiece& aPiece, TInt aBegin, TInt aLength) = 0;
};

/**
 * TPieceWriteEntry
 */
class CPieceWriteEntry : public CBase
{
public:
	
	class TSTFilePos
	{
	public:
		TSTFilePos(CSTFile* aFile, TInt aPos)
		 : iFile(aFile), iPos(aPos) {}
		
		TBool operator==(const TSTFilePos& aPos)
		{ return ((aPos.iPos == iPos) && (aPos.iFile == iFile)); }

	public:
		CSTFile* iFile;
		TInt iPos;
	};
	
	CPieceWriteEntry(CSTPiece& aPiece, TInt aBegin, TInt aLength, MPieceWriteObserver* aPieceWriteObserver, CSTPeer* aPeer = NULL)
	 : iPiece(aPiece), iPeer(aPeer), iBegin(aBegin), iLength(aLength), iPieceWriteObserver(aPieceWriteObserver)
	{ iWriteCompleteCount = 0; }
	
	~CPieceWriteEntry() { iFiles.Reset(); }
	
public:
	CSTPiece& iPiece;
	CSTPeer* iPeer;
	TInt iBegin;
	TInt iLength;
	
	TInt iWriteCompleteCount;
	
	MPieceWriteObserver* iPieceWriteObserver;
	
	RArray<TSTFilePos> iFiles;
};

/**
 *  CSTPieceAccess
 */
class CSTPieceAccess : public CBase, public MSTFileWriteObserver
{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CSTPieceAccess();

	/**
	 * Two-phased constructor.
	 */
	static CSTPieceAccess* NewL(CSTFileManager& aFileManager);

	/**
	 * Two-phased constructor.
	 */
	static CSTPieceAccess* NewLC(CSTFileManager& aFileManager);
	
	void WritePieceAsyncL(CSTPiece& aPiece, TInt iBegin, const TDesC8& aData, MPieceWriteObserver* aPieceWriteObserver, CSTPeer* aPeer = NULL);
	
	/**
	 * Cancels all write operations that are associated with the given peer
	 */
	void CancelAllWrite(CSTPeer* aPeer);

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CSTPieceAccess(CSTFileManager& aFileManager);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	
	/**
	 * Gets the index of the piece writer that is associated with the given file operation
	 */
	TInt GetWriterIndex(CSTFile& aFile, TInt aPosition);
	
private: // from MSTFileWriteObserver
	
	void OnFileWriteCompleteL(CSTFile& aFile, TInt aPosition);
	
	void OnFileWriteFailedL(CSTFile& aFile, TInt aPosition);
	
private:
	
	RPointerArray<CPieceWriteEntry> iWriters;
	
	CSTFileManager& iFileManager;

};

#endif // STPIECEACCESS_H
