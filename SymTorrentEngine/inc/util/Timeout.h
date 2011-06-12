#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

// ============================ INCLUDE FILES ==================================
#include <e32base.h>


// ============================ FORWARD DECLARATIONS ===========================
class CTimeout;


// ============================ INTERFACE DECLARATIONS =========================
class MTimeoutObserver
    {
public:
    /*
     * Called when the specified time has elapsed.
     * If it returns ETrue the timer will be reactivated.
     */
    virtual TBool HandleTimeIsUp(CTimeout* aTimeout, TInt aStatus) = 0;
    };


// ============================ CLASS DECLARATIONS =============================
class CTimeout : public CActive
    {
public:
    static CTimeout* NewL(MTimeoutObserver* aObserver, TInt aPriority = CActive::EPriorityStandard);
    ~CTimeout();
    
private:
    CTimeout(MTimeoutObserver* aObserver, TInt aPriority);
    void ConstructL();

public: // from CActive
    virtual void RunL();
    virtual void DoCancel();
    virtual TInt RunError(TInt aError);

public:
    /*
     * aTime in milisec
     */
    void Start(TInt64 aTime = 0);
    void Stop();
    void Reset(TInt64 aTime = 0);
    
    /**
     * Restarts the timer with the current (previously set) timeout
     */
    void Restart();

private: // data members
    MTimeoutObserver*   iObserver;
    RTimer              iTimer;
    TInt64              iTime;
    };

#endif
