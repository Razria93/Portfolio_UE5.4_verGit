#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

namespace PortfolioRecording
{
// Keep console-origin settings at their existing priority. Do not overwrite a
// newer user edit made while capture was active.
struct FCVarOverride
{
    FString Name, Previous, Applied;
    EConsoleVariableFlags Priority = ECVF_SetByConstructor;

    void Apply(const TCHAR* InName, int32 Value)
    {
        Restore();
        if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(InName))
        {
            Name = InName;
            Previous = Variable->GetString();
            Priority = static_cast<EConsoleVariableFlags>(Variable->GetFlags() & ECVF_SetByMask);
            Variable->Set(Value, Priority);
            Applied = Variable->GetString();
        }
    }

    void Restore()
    {
        if (!Name.IsEmpty())
        {
            if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name))
            {
                if ((Variable->GetFlags() & ECVF_SetByMask) == uint32(Priority) && Variable->GetString() == Applied)
                    Variable->Set(*Previous, Priority);
            }
            Name.Reset();
        }
    }
};

// A take has one fixed viewport and crop. Moving the desktop window alone
// does not change its window-local crop; resizing/reparenting/layout does.
struct FRecordingGeometry
{
    FIntPoint ViewportSize = FIntPoint::ZeroValue;
    FIntPoint CropOrigin = FIntPoint::ZeroValue;
    FIntPoint BackBufferSize = FIntPoint::ZeroValue;
    UPTRINT WindowId = 0;

    bool Matches(const FRecordingGeometry& Other) const
    {
        return WindowId != 0 && ViewportSize.X > 0 && ViewportSize.Y > 0
            && BackBufferSize.X > 0 && BackBufferSize.Y > 0
            && ViewportSize == Other.ViewportSize && CropOrigin == Other.CropOrigin
            && BackBufferSize == Other.BackBufferSize && WindowId == Other.WindowId;
    }
};

// Polled on the editor ticker: no game-thread sleep while JPEG/WAV workers run.
struct FExportProgress
{
    static constexpr double TimeoutSeconds = 15.0;
    double StartedAt = 0;
    int64 PreviousAudioSize = -1;
    int32 StableAudioChecks = 0;

    bool Poll(double Now, int32 PendingFrames, int64 AudioSize)
    {
        StableAudioChecks = AudioSize > 44 && AudioSize == PreviousAudioSize ? StableAudioChecks + 1 : 0;
        PreviousAudioSize = AudioSize;
        return HasTimedOut(Now) || (PendingFrames == 0 && HasStableAudio());
    }
    bool HasTimedOut(double Now) const { return Now - StartedAt >= TimeoutSeconds; }
    bool HasStableAudio() const { return StableAudioChecks >= 3; }
};
}
