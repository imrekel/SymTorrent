/*
 ============================================================================
 Name		: MeasurementLog.cpp
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CMeasurementLog implementation
 ============================================================================
 */

#include "MeasurementLog.h"
#include "STTorrent.h"

_LIT(KFinalLogFileName, "c:\\data\\GridTorrent-final-LOG.txt");
_LIT(KTorrentCompleteLogFileName, "c:\\data\\GridTorrent-torrent_downloaded-LOG.txt");

CMeasurementLog::CMeasurementLog()
{
	// No implementation required
}

CMeasurementLog::~CMeasurementLog()
{
}

CMeasurementLog* CMeasurementLog::NewLC()
{
	CMeasurementLog* self = new (ELeave) CMeasurementLog();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

CMeasurementLog* CMeasurementLog::NewL()
{
	CMeasurementLog* self = CMeasurementLog::NewLC();
	CleanupStack::Pop(); // self;
	return self;
}

void CMeasurementLog::SaveLogL(CSTTorrent* aTorrent, const TDesC& aFileName)
{
	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);
	
	_LIT8(KLitSeparator, ";");

	RFile file;
	if (file.Replace(fs, aFileName, EFileWrite) == KErrNone)
	{
		CleanupClosePushL(file);
		
		TBuf8<32> numBuf;
		
		numBuf.Num(aTorrent->iSentLocalRequests);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalRequestTimeouts);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalSubPiecesForRequests);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalSubPiecesNotRequested);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalSubPiecesReceivedAlreadyDownloaded);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalSubPiecesForRequestsSize);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalSubPiecesNotRequestedSize);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalSubPiecesReceivedAlreadyDownloadedSize);
		file.Write(numBuf);
		file.Write(KLitSeparator);
				
		numBuf.Num(aTorrent->iLocalSubPiecesReceivedNotMatchPieceBeginning);
		file.Write(numBuf);
		file.Write(KLitSeparator);
				
		numBuf.Num(aTorrent->iLocalBytesUploaded);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLocalBytesDownloaded);
		file.Write(numBuf);
		file.Write(KLitSeparator);
				
		numBuf.Num(aTorrent->iLongRangeBytesUploaded);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLongRangeBytesDownloaded);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLongRangeSubPiecesReceived);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->iLongRangeSubPiecesReceivedSize);
		file.Write(numBuf);
		file.Write(KLitSeparator);
		
		numBuf.Num(aTorrent->EllapsedTime());
		file.Write(numBuf);
		
		file.Write(_L8("\r\n"));
			
		CleanupStack::PopAndDestroy(); // file
	}
	
	CleanupStack::PopAndDestroy(); // fs
}

EXPORT_C void CMeasurementLog::SavePreliminaryLogL(CSTTorrent* aTorrent)
{
	SaveLogL(aTorrent, KTorrentCompleteLogFileName);
}

EXPORT_C void CMeasurementLog::SaveFinalLogL(CSTTorrent* aTorrent)
{
	SaveLogL(aTorrent, KFinalLogFileName);
}

void CMeasurementLog::ConstructL()
{

}
