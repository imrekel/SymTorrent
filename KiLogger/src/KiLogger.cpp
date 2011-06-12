#define LOG_TO_FILE

#include "KiLogger.h"
#include <txtetext.h>

#ifdef EKA2
	#ifdef __WINS__
 		_LIT(KLogFileName, "c:\\SymTorrent.log");
 	#else
 		_LIT(KLogFileName, "e:\\SymTorrentLOG.txt");
 	#endif
#else
 _LIT(KLogFileName, "c:\\SymTorrent.log");
#endif

_LIT8(KKiNewLine, "\r\n");

CKiLogger::CKiLogger(TUid aUid, RFs& aFsSession, TTimeStampMode aTimeStampMode)
 : iUid(aUid), iNewLine(ETrue), iFsSession(aFsSession), iTimeStampMode(aTimeStampMode)
{
	iStartTime.HomeTime();
}

void CKiLogger::ConstructL(const TDesC& aLogFileName)
{

#ifdef LOG_TO_FILE
/*	User::LeaveIfError(iFsSession.Connect());
	if (iLogFile.Open(iFsSession, KLogFileName, EFileWrite) != KErrNone)
		User::LeaveIfError(iLogFile.Create(iFsSession, KLogFileName, EFileWrite));
	else
	{
		TInt pos = 0;
		iLogFile.Seek(ESeekEnd, pos);
	}*/
#endif

	if (aLogFileName != KNullDesC)
		SetLogFileL(aLogFileName);
	else
	{
		TBuf<KMaxUidName + 15> logFileName;
		_LIT(KCDrive, "c:\\");
		logFileName.Append(KCDrive);
		logFileName.Append(iUid.Name());
		_LIT(KLogTxt, "-LOG.txt");
		logFileName.Append(KLogTxt);

		SetLogFileL(logFileName);
	}
}


CKiLogger::~CKiLogger()
{

#ifdef LOG_TO_FILE
	iLogFile.Close();
#endif

	delete iLogFileName;
}

EXPORT_C void CKiLogger::SetLogFileL(const TDesC& aLogFileName)
{
#ifdef LOG_TO_FILE
	if (iLogFileName)
	{
		if (*iLogFileName != aLogFileName) // checks whether the given filename differs from the name of the current log file
		{
			iLogFile.Close();
			
			if (iLogFile.Open(iFsSession, aLogFileName, EFileWrite) == KErrNone)
			{
				TInt pos = 0;
				iLogFile.Seek(ESeekEnd, pos);
			}
			else
				User::LeaveIfError(iLogFile.Create(iFsSession, aLogFileName, EFileWrite));
			
			RFile oldLogFile;
			if (oldLogFile.Open(iFsSession, *iLogFileName, EFileRead) == KErrNone)
			{
				CleanupClosePushL(oldLogFile);
				HBufC8* readBuf = HBufC8::NewLC(512);
				TPtr8 readBufPtr(readBuf->Des());
				
				// copies the content of the old log file to the newly created one
				oldLogFile.Read(readBufPtr);
				while (readBufPtr.Length() != 0)
				{
					iLogFile.Write(*readBuf);
					
					readBufPtr.SetLength(0);
					oldLogFile.Read(readBufPtr);
				}
				
				CleanupStack::PopAndDestroy(readBuf);
				CleanupStack::PopAndDestroy(); // oldLogFile
			}
		}
	}
	else
	{
		if (iLogFile.Open(iFsSession, aLogFileName, EFileWrite) != KErrNone)
			User::LeaveIfError(iLogFile.Create(iFsSession, aLogFileName, EFileWrite));
		
		TInt pos = 0;
		iLogFile.Seek(ESeekEnd, pos);
	}
#endif // LOG_TO_FILE

	delete iLogFileName;
	iLogFileName = NULL;
	iLogFileName = aLogFileName.AllocL();		
}

EXPORT_C void CKiLogger::WriteL(const TDesC& aText)
{
#ifdef LOG_TO_FILE
	HBufC8* buf = HBufC8::NewLC(aText.Length());
	TPtr8 ptr(buf->Des());
	ptr.Copy(aText);	
	WriteL(ptr);
	CleanupStack::PopAndDestroy(buf);
#endif
}

EXPORT_C void CKiLogger::WriteL(TInt64 aInt)
{
	TBuf8<32> buf;
	buf.Num(aInt);
	WriteL(buf);
}

EXPORT_C void CKiLogger::WriteLineL(TInt64 aInt)
{
	WriteL(aInt);
	WriteLineL();
}


EXPORT_C void CKiLogger::WriteL(const TDesC8& aText)
{
#ifdef LOG_TO_FILE

	if (iNewLine)
	{
		WriteTimeStampL();
		iNewLine = EFalse;
	}
	
	if (FreeBufferSize() >= aText.Length())
	{
		iLogBuffer.Append(aText);
	}
	else
	{
		TInt processedLength = FreeBufferSize();
		iLogBuffer.Append(aText.Left(processedLength));
		FlushBuffer();
		WriteL(aText.Mid(processedLength));
	}	
#endif
}

EXPORT_C void CKiLogger::WriteLineL(const TDesC8& aText)
{
	WriteL(aText);
	WriteLineL();
}

EXPORT_C void CKiLogger::WriteLineL()
{
	WriteL(KKiNewLine);
	FlushBuffer();
	iNewLine = ETrue;
}

EXPORT_C void CKiLogger::WriteLineL(const TDesC& aText)
{
	WriteL(aText);
	WriteLineL();
}

EXPORT_C void CKiLogger::WriteTimeStampL()
{
	TTime now;
	now.HomeTime();
	TBuf8<30> timeBuf;
	
	
	if (iTimeStampMode == ESystemTime)
	{
		TDateTime dateTime(now.DateTime());
		
		timeBuf.Append(_L("["));
		timeBuf.AppendNum(dateTime.Hour());
		timeBuf.Append(TChar(':'));
		timeBuf.AppendNum(dateTime.Minute());
		timeBuf.Append(TChar(':'));
		timeBuf.AppendNum(dateTime.Second());
		timeBuf.Append(_L("] "));
	}
	else
	{
		TTimeIntervalSeconds seconds;
		now.SecondsFrom(iStartTime, seconds);
		
		TTimeIntervalMinutes minutes;
		now.MinutesFrom(iStartTime, minutes);
		
		TTimeIntervalHours hours;
		now.HoursFrom(iStartTime, hours);
		
		timeBuf.Append(_L("["));
		timeBuf.AppendNum(hours.Int());
		timeBuf.Append(TChar(':'));
		timeBuf.AppendNum(minutes.Int() % 60);
		timeBuf.Append(TChar(':'));
		timeBuf.AppendNum(seconds.Int() % 60);
		timeBuf.Append(_L("] "));
	}	
	
	if (timeBuf.Length() > FreeBufferSize())
		FlushBuffer();
	
	iLogBuffer.Append(timeBuf);
}

EXPORT_C void CKiLogger::FlushBuffer()
{
	iLogFile.Write(iLogBuffer);
	iLogBuffer.SetLength(0);
}



/*EXPORT_C void CKiLogger::WriteL(const TDesC& aText)
{

#ifdef LOG_TO_FILE
	HBufC8* buf = HBufC8::NewLC(aText.Length());
	TPtr8 ptr(buf->Des());
	ptr.Copy(aText);	
	iLogFile.Write(ptr);
	CleanupStack::PopAndDestroy();
#endif
}


EXPORT_C void CKiLogger::WriteL(const TDesC8& aText)
{

#ifdef LOG_TO_FILE
	iLogFile.Write(aText);
#endif
}


EXPORT_C void CKiLogger::WriteLineL(const TDesC& aText)
{
	WriteL(aText);
	WriteLineL();
}


EXPORT_C void CKiLogger::WriteLineL(const TDesC8& aText)
{
	WriteL(aText);
	WriteLineL();
}


EXPORT_C void CKiLogger::WriteLineL()
{
#ifdef LOG_TO_FILE
	iLogFile.Write(_L8("\r\n"));
#endif
}


EXPORT_C void CKiLogger::WriteL(TInt aInt)
{
	TBuf<12> buf;
	buf.Num(aInt);
	WriteL(buf);
}


EXPORT_C void CKiLogger::WriteLineL(TInt aInt)
{
	WriteL(aInt);
	WriteLineL();
}*/
