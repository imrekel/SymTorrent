/*
 ============================================================================
 Name		: STPieceAccess.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CSTPieceAccess implementation
 ============================================================================
 */

#include "STPieceAccess.h"
#include "STPiece.h"
#include "STFile.h"
#include "ST.pan"
#include "SymTorrentEngineLog.h"
#include "STDefs.h"

CSTPieceAccess::CSTPieceAccess(CSTFileManager& aFileManager)
 : iFileManager(aFileManager)
{
	// No implementation required
}

CSTPieceAccess::~CSTPieceAccess()
{
	iWriters.ResetAndDestroy();
}

CSTPieceAccess* CSTPieceAccess::NewLC(CSTFileManager& aFileManager)
{
	CSTPieceAccess* self = new (ELeave) CSTPieceAccess(aFileManager);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CSTPieceAccess* CSTPieceAccess::NewL(CSTFileManager& aFileManager)
{
	CSTPieceAccess* self=CSTPieceAccess::NewLC(aFileManager);
	CleanupStack::Pop(); // self;
	return self;
}

void CSTPieceAccess::ConstructL()
{

}

TInt CSTPieceAccess::GetWriterIndex(CSTFile& aFile, TInt aPosition)
{
	for (TInt i=0; i<iWriters.Count(); i++)
	{
		for (TInt j=0; j<iWriters[i]->iFiles.Count(); j++)
			if (iWriters[i]->iFiles[j] == CPieceWriteEntry::TSTFilePos(&aFile, aPosition))
				return i;
	}
	
	return -1;
}

void CSTPieceAccess::OnFileWriteCompleteL(CSTFile& aFile, TInt aPosition)
{
	LWRITE(LOG, _L("Write complete in: "));
	LWRITE(LOG, aFile.Path());
	LWRITE(LOG, _L(" "));
	LWRITELN(LOG, aPosition);
	
	TInt writerIndex = GetWriterIndex(aFile, aPosition);
	if (writerIndex < 0)
		return;
	CPieceWriteEntry* writer = iWriters[writerIndex];
	
	if (writer)
		writer->iWriteCompleteCount++;
	
	if (writer->iFiles.Count() == writer->iWriteCompleteCount)
	{
		writer->iPiece.IncreaseDownloadedSizeL(writer->iLength, writer->iPeer);
		
		TInt begin = writer->iBegin;
		TInt length = writer->iLength;
		CSTPiece& piece = writer->iPiece;
		
		delete writer;
		iWriters.Remove(writerIndex);
		
		writer->iPieceWriteObserver->OnPieceWriteCompleteL(piece, begin, length);		
	}
}
	
void CSTPieceAccess::OnFileWriteFailedL(CSTFile& aFile, TInt aPosition)
{
	LWRITE(LOG, _L("Write failed in: "));
	LWRITE(LOG, aFile.Path());
	LWRITE(LOG, _L(" "));
	LWRITELN(LOG, aPosition);
	
	TInt writerIndex = GetWriterIndex(aFile, aPosition);
	if (writerIndex < 0)
		return;
	CPieceWriteEntry* writer = iWriters[writerIndex];
		
	if (writer)
	{
		writer->iPieceWriteObserver->OnPieceWriteFailedL(writer->iPiece, writer->iBegin, writer->iLength);
		delete writer;
		iWriters.Remove(writerIndex);
	}		
}

void CSTPieceAccess::CancelAllWrite(CSTPeer* aPeer)
{
	for (TInt i=0; i<iWriters.Count(); i++)
	{
		if (iWriters[i]->iPeer == aPeer)
		{
			delete iWriters[i];
			iWriters.Remove(i);
			i--;
		}
	}
}

void CSTPieceAccess::WritePieceAsyncL(CSTPiece& aPiece, TInt aBegin, const TDesC8& aBlock, MPieceWriteObserver* aPieceWriteObserver, CSTPeer* aPeer)
{
	TInt blockPosition = 0;
	TInt piecePosition = aBegin;
	
	LWRITE(LOG, _L("Writing to piece started, piece: "));
	LWRITE(LOG, aPiece.Index());
	LWRITE(LOG, _L(" Position: "));
	LWRITELN(LOG, piecePosition);
	
	CPieceWriteEntry* writer = 
		new (ELeave) CPieceWriteEntry(aPiece, aBegin, aBlock.Length(), aPieceWriteObserver, aPeer);
	CleanupStack::PushL(writer);
	iWriters.AppendL(writer);
	CleanupStack::Pop(); // writer

	while (blockPosition < aBlock.Length())
	{
		CSTFile* file = NULL;
		TInt filePosition = 0;

		aPiece.GetFilePosition(piecePosition, &file, filePosition);

		if (file)
		{
			writer->iFiles.AppendL(CPieceWriteEntry::TSTFilePos(file, filePosition));
			
			//TInt res = KErrNone;
			TInt fileBlockLength = file->Size() - filePosition;

			//LogPieceIndexL();
			LWRITE(LOG, _L("Appending block to "));
			LWRITE(LOG, file->Path());
			LWRITE(LOG, _L(" "));
			LWRITELN(LOG, filePosition);

			if (fileBlockLength <= (aBlock.Length() - blockPosition))	// reached the end of a file
			{
				//LogPieceIndexL();
				LWRITE(LOG, _L("File complete: "));
				LWRITELN(LOG, file->Path());
				
				iFileManager.WriteAsyncL(file, 
					filePosition, aBlock.Mid(blockPosition, fileBlockLength), this);
				
				//FileManager().Close(file);

				blockPosition += fileBlockLength;
				piecePosition += fileBlockLength;				
			}
			else // the file isn't finished yet, the complete block is appended to the file
			{
				iFileManager.WriteAsyncL(file, 
					filePosition, aBlock.Right(aBlock.Length() - blockPosition), this);												

				piecePosition += (aBlock.Length() - blockPosition);
				//blockPosition = aBlock.Length();
				break;
			}
		}
		else
			User::Panic(KSymTorrentEnginePanic, KPanGetFilePositionFailed);
	}
}
