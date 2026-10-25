#pragma once
#include "CoreMinimal.h"

namespace PortfolioReplay
{
// Classification of the first fixture attack only, not a gameplay decision.
struct FParryReplayDiagnostic
{
    uint64 Generation = 0;
    int32 Index = INDEX_NONE;
    bool bWindowOpened = false, bWindowClosed = false;
    bool bContact = false, bParryOpenAtContact = false;
    bool bAccepted = false, bEligiblePolicy = false, bParryResult = false;
    uint64 ContactFrame = 0;
    int32 ContactWindowId = INDEX_NONE;
    // An unaccepted overlap is pending evidence; a later contact may replace it.
    bool RecordContact(uint64 InGeneration, int32 InIndex, uint64 Frame, int32 Window, bool bParryOpen)
    {
        if (bAccepted || !Matches(InGeneration, InIndex) || Window == INDEX_NONE) return false;
        bContact = true; bParryOpenAtContact = bParryOpen;
        ContactFrame = Frame; ContactWindowId = Window;
        return true;
    }
    bool RecordAccepted(uint64 InGeneration, int32 InIndex, uint64 Frame, int32 Window,
        bool bResultAccepted, bool bPolicyEligible, bool bResultParry)
    {
        if (bAccepted || !bContact || !Matches(InGeneration, InIndex)
            || ContactFrame != Frame || ContactWindowId != Window || !bResultAccepted) return false;
        bAccepted = true; bEligiblePolicy = bPolicyEligible; bParryResult = bResultParry;
        return true;
    }
    bool CanComplete(bool bObservationComplete) const
    {
        return bObservationComplete && Generation != 0 && Index == 0 && bContact && bAccepted
            && bEligiblePolicy && bParryOpenAtContact && bParryResult;
    }
    bool Matches(uint64 InGeneration, int32 InIndex) const
    { return Generation != 0 && Generation == InGeneration && Index == InIndex; }
    const TCHAR* Classify() const
    {
        if (Generation == 0) return TEXT("IncompleteObservation");
        if (!bContact) return bWindowOpened && bWindowClosed ? TEXT("NoContact") : TEXT("IncompleteObservation");
        if (!bAccepted) return TEXT("ContactUnresolved");
        if (!bEligiblePolicy) return TEXT("OtherDefensePolicy");
        if (bParryOpenAtContact != bParryResult) return TEXT("GateMismatch");
        return bParryResult ? TEXT("ParryConfirmed") : TEXT("ContactOutsideParryWindow");
    }
};
}
