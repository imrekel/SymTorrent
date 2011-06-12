#ifndef KILOGGER_H__
#define KILOGGER_H__

//#include "KiLog.h"

#ifdef LOG_TO_FILE
#define LWRITE(log, str) log->WriteL(str)
#define LWRITELN(log, str) log->WriteLineL(str)
#define LLINE(log) log->WriteLineL()
#else
#define LWRITE(log, str)
#define LWRITELN(log, str)
#define LLINE(log)
#endif

#ifdef HEAVY_LOG
#define HLWRITE(log, str) log->WriteL(str)
#define HLWRITELN(log, str) log->WriteLineL(str)
#define HLLINE log->WriteLineL()
#else
#define HLWRITE(log, str)
#define HLWRITELN(log, str)
#define HLLINE(log)
#endif

#include <coemain.h>

class RFs;
class CKiLogger;

#ifdef LOG_TO_FILE

const TUid KUidKiLoggerSingleton = { 0x10000EF4 };
const TInt KKiLogBufferSize = 128;


enum TTimeStampMode
{
	ESystemTime,
	ETimePassedSinceStart
};

/**
 * CKiLogger
 */
class CKiLogger : public CCoeStatic
{
public:

	IMPORT_C CKiLogger(TUid aUid, RFs& aFsSession, TTimeStampMode aTimeStampMode = ESystemTime);
	
	IMPORT_C void ConstructL(const TDesC& aLogFileName = KNullDesC);
	
	IMPORT_C ~CKiLogger();
	
	/**
	 * @return the logger's associated UID
	 */
	inline TUid Uid() const;
	
	/**
	 * Sets/changes the associated log file. If a new filename is defined,
	 * then the content of the old log file is copied to the end of new one (the content
	 * of both files is preserved).
	 */
	IMPORT_C void SetLogFileL(const TDesC& aLogFileName);

	IMPORT_C void WriteL(const TDesC& aText);

	IMPORT_C void WriteL(const TDesC8& aText);

	IMPORT_C void WriteLineL(const TDesC& aText);

	IMPORT_C void WriteLineL(const TDesC8& aText);

	IMPORT_C void WriteLineL();

	IMPORT_C void WriteL(TInt64 aInt);

	IMPORT_C void WriteLineL(TInt64 aInt);
	
	/**
	 * Appends a time stamp to the log
	 */
	IMPORT_C void WriteTimeStampL();
	
	/**
	 * Writes the content of the log buffer to the log file and resets the buffer.
	 */
	IMPORT_C void FlushBuffer();
	
private:
	
	/**
	 * @return the number of remaining (free) bytes in the log buffer
	 */
	inline TInt FreeBufferSize() const;

private:

	TBuf8<KKiLogBufferSize> iLogBuffer;
	
	/**
	 * UID of the log, usually the same as the caller application's UID
	 */
	TUid iUid;
	
	TBool iNewLine;

	/**
	 * Reference to a file server session, owned elsewhere
	 */
	RFs& iFsSession;	
	
	RFile iLogFile;
	
	HBufC* iLogFileName;
	
	TTime iStartTime;
	
	TTimeStampMode iTimeStampMode;
};

// INLINE DEFINITIONS

inline TUid CKiLogger::Uid() const {
	return iUid;
}

inline TInt CKiLogger::FreeBufferSize() const {
	return KKiLogBufferSize - iLogBuffer.Length() - 2;
}

#endif

#endif
