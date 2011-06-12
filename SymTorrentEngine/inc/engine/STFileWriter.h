/*
 ============================================================================
 Name		: STFileWriter.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CSTFileWriter declaration
 ============================================================================
 */

#ifndef STFILEWRITER_H
#define STFILEWRITER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

class CSTFileManagerEntry;
class MSTFileWriteObserver;

// CLASS DECLARATION

/**
 *  CSTFileWriter
 * 
 */
class CSTFileWriter : public CActive
{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CSTFileWriter();

	/**
	 * Two-phased constructor.
	 */
	static CSTFileWriter* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CSTFileWriter* NewLC();
	
	void StartWrite(CSTFileManagerEntry* iFileMgrEntry, 
			        TInt aPosition, 
			        const TDesC8& aData, 
			        MSTFileWriteObserver* aObserver);
	
private: // from CActive
	
	void RunL();
	
	void DoCancel();

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CSTFileWriter();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	
	MSTFileWriteObserver* iObserver;
	
	CSTFileManagerEntry* iFileMgrEntry;
	
	TInt iPosition;

};

#endif // STFILEWRITER_H
