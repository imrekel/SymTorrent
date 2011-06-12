#ifndef KILOGMANAGER_H__
#define KILOGMANAGER_H__

#include <coemain.h>
#include "KiLogger.h"

class CKiLogManager;

#ifdef LOG_TO_FILE

#define LOGMGR CKiLogManager::Instance()

//const TUid KUidKiLogManagerSingleton = { 0x10000EF4 };

/**
 * CKiLogManager
 *
 * Singleton. Supervises multiple "loggers" (log files).
 */
class CKiLogManager : public CBase
{
public:

	IMPORT_C static CKiLogManager* Instance();	
	
	/**
	 * Initializes the log manager in case it is not initialized yet. 
	 * 
	 * This can be called several times, even if the log manager is already initialized.
	 * However, the class uses reference counting for cleanup; thus, every call to initialize 
	 * must be followed by a call to Free() at some time. 
	 */
	IMPORT_C static void InitializeL();
	
	/**
	 * Frees up one reference
	 */
	IMPORT_C static void Free();
	
	~CKiLogManager();
	
	/**
	 * @return the logger with the given UID
	 */
	IMPORT_C CKiLogger* GetLoggerL(TUid aAppUid);
	
	/**
	 * Creates a new logger with the given UID and file name.
	 *
	 * If a logger with the same UID is already exists, the functions panics.
	 */
	IMPORT_C CKiLogger* CreateLoggerL(TUid aUid, const TDesC& aLogFileName, TTimeStampMode aTimeStampMode = ESystemTime);
	
private:

	CKiLogManager();
	
	void ConstructL();
	
private:

	RPointerArray<CKiLogger> iLoggers;
	
	RFs iFs;
	
	TInt iReferenceCount;

};

#endif

#endif
