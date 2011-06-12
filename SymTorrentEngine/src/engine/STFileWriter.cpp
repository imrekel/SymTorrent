/*
 ============================================================================
 Name		: STFileWriter.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CSTFileWriter implementation
 ============================================================================
 */

#include "STFileWriter.h"
#include "STFileManagerEntry.h"
#include "STFileManager.h"

CSTFileWriter::CSTFileWriter() : CActive(EPriorityStandard)
{
	CActiveScheduler::Add(this);
}

CSTFileWriter::~CSTFileWriter()
{
	Cancel();
}

CSTFileWriter* CSTFileWriter::NewLC()
{
	CSTFileWriter* self = new (ELeave)CSTFileWriter();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CSTFileWriter* CSTFileWriter::NewL()
{
	CSTFileWriter* self=CSTFileWriter::NewLC();
	CleanupStack::Pop(); // self;
	return self;
}

void CSTFileWriter::ConstructL()
{
}

void CSTFileWriter::RunL()
{
	switch (iStatus.Int())
	{
		case KErrNone:
			iObserver->OnFileWriteCompleteL(iFileMgrEntry->File(), iPosition);
		break;
		
		default:
			iObserver->OnFileWriteFailedL(iFileMgrEntry->File(), iPosition);
		break;
	}
	
	// Delete the active object
	iFileMgrEntry->OnFileWriteCompleteL(this);
}
	
void CSTFileWriter::DoCancel()
{
	// No way to cancel file write requests. Empty DoCancel waits for the request to complete.
}

void CSTFileWriter::StartWrite(	CSTFileManagerEntry* aFileMgrEntry, 
			        			TInt aPosition, 
			        			const TDesC8& aData, 
			        			MSTFileWriteObserver* aObserver)
{
	iPosition = aPosition;
	iObserver = aObserver;
	iFileMgrEntry = aFileMgrEntry;
	
	iFileMgrEntry->FileHandle().Write(iPosition, aData, iStatus);
	SetActive();
}
