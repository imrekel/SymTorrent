// ============================ INCLUDE FILES ==================================
#include "Timeout.h"

// ============================ MEMBER FUNCTIONS ===============================
CTimeout* CTimeout::NewL(MTimeoutObserver* aObserver, TInt aPriority)
    {
    CTimeout* self = new (ELeave) CTimeout(aObserver, aPriority);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }


CTimeout::CTimeout(MTimeoutObserver* aObserver, TInt aPriority)
:   CActive(aPriority),
    iObserver(aObserver)
    {
    }


CTimeout::~CTimeout()
    {
    Cancel(); // cancel ANY outstanding request at time of destruction
    iTimer.Close();
    }


void CTimeout::ConstructL()
    {
    CActiveScheduler::Add(this);
    User::LeaveIfError(iTimer.CreateLocal());
    }


void CTimeout::RunL()
    {
    if (iObserver->HandleTimeIsUp(this, iStatus.Int()))
        {
        Start();
        }
    }


void CTimeout::DoCancel()
    {
    iTimer.Cancel();
    }

    
TInt CTimeout::RunError(TInt aError)
    {
    return aError;
    }


void CTimeout::Start(TInt64 aTime)
    {
//    TRACE_FUNCF((_L("aTime=%d iTime=%d"), aTime, iTime));
//    LOG_DEBUGF((_L("start timer aTime=%d iTime=%d"), aTime, iTime));
    if (aTime != 0)
        {
        iTime = aTime;
        }
    if (iTime < 2100000)
        {
        iTimer.After(iStatus, iTime * 1000);
        }
    else
        {
        TTime time;
        time.HomeTime();
        TTimeIntervalMicroSeconds delta = iTime * 1000;
        time += delta;
        iTimer.At(iStatus, time);
        }
    SetActive();
    }

    
void CTimeout::Stop()
    {
    Cancel();
    }

void CTimeout::Reset(TInt64 aTime)
    {
    Stop();
    Start(aTime);
    }

void CTimeout::Restart()
	{
	Reset(iTime);
	}
