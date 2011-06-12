#define LOG_TO_FILE

#include "KiLogManager.h"
#include "KiLogger.h"
#include "KiLog.pan"

_LIT(KLitKiLogger, "KiLogger");

CKiLogManager::CKiLogManager()
{
}

void CKiLogManager::ConstructL()
{
	iFs.Connect();
}

CKiLogManager::~CKiLogManager()
{
	iLoggers.ResetAndDestroy();
	iFs.Close();
}


EXPORT_C CKiLogManager* CKiLogManager::Instance()
{
	return (CKiLogManager*)Dll::Tls();
}

EXPORT_C void CKiLogManager::InitializeL()
{
	CKiLogManager* instance = static_cast<CKiLogManager*>(Dll::Tls());
	
	if (instance == 0)
	{
		instance = new (ELeave) CKiLogManager();
		CleanupStack::PushL( instance ); 
		instance->ConstructL(); 
		CleanupStack::Pop();				
		
		Dll::SetTls(instance);
	}
	
	instance->iReferenceCount++;
}

EXPORT_C void CKiLogManager::Free()
{
	CKiLogManager* instance = (CKiLogManager*)Dll::Tls();
	
	if (instance)
	{
		instance->iReferenceCount--;
		
		if (instance->iReferenceCount == 0)
		{
			delete (CKiLogManager*)Dll::Tls();
			Dll::SetTls(NULL);
		}
	}
	else
		User::Panic(KLitKiLogPanic, ELogManagerFreedUpTooManyTimes);
}

EXPORT_C CKiLogger* CKiLogManager::GetLoggerL(TUid aUid)
{
	for (TInt i=0; i<iLoggers.Count(); i++)
	{
		if (iLoggers[i]->Uid() == aUid)
			return iLoggers[i];
	}
	
	CKiLogger* logger = new (ELeave) CKiLogger(aUid, iFs);
	CleanupStack::PushL(logger);
	logger->ConstructL();
	User::LeaveIfError(iLoggers.Append(logger));
	CleanupStack::Pop(); // logger
	
	return logger;
}

EXPORT_C CKiLogger* CKiLogManager::CreateLoggerL(TUid aUid, const TDesC& aLogFileName, TTimeStampMode aTimeStampMode)
{
	for (TInt i=0; i<iLoggers.Count(); i++)
	{
		if (iLoggers[i]->Uid() == aUid)
			User::Panic(KLitKiLogger, 0);
	}
	
	CKiLogger* logger = new (ELeave) CKiLogger(aUid, iFs, aTimeStampMode);
	CleanupStack::PushL(logger);
	logger->ConstructL(aLogFileName);
	User::LeaveIfError(iLoggers.Append(logger));
	CleanupStack::Pop(); // logger
	
	return logger;
}
