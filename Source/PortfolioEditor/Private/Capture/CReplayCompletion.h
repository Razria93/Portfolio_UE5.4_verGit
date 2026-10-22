#pragma once

#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PortfolioReplay
{
// Record the requested gameplay goal separately from recorder file completion.
inline FString ValidateScenarioGoal(const FString& Mode, bool bEquipped, bool bUnequipped, int32 MaxComboIndex)
{
    if (Mode == TEXT("equip") && (!bEquipped || !bUnequipped)) return TEXT("EquipmentTransitionNotObserved");
    const int32 RequiredHits = Mode == TEXT("combo2") ? 2 :
        ((Mode == TEXT("combo") || Mode == TEXT("combo3")) ? 3 : 0);
    if (RequiredHits > 0 && MaxComboIndex < RequiredHits - 1) return TEXT("ComboGoalNotReached");
    return FString();
}

// Busy means export is pending, not that it succeeded. This deliberately
// requires a complete take; dropped frames are not silently accepted.
inline FString ValidateCompletion(const FString& DoneText, const FString& SummaryText, bool bAllowSilentAudio = false)
{
    auto Parse = [](const FString& Text, TMap<FString, FString>& Values)
    {
        TArray<FString> Lines;
        Text.ParseIntoArrayLines(Lines);
        for (const FString& Line : Lines)
        {
            FString Key, Value;
            if (!Line.Split(TEXT("="), &Key, &Value)) continue;
            if (Values.Contains(Key)) return false;
            Values.Add(Key, Value);
        }
        return true;
    };
    TMap<FString, FString> Done, Summary;
    if (!Parse(DoneText, Done) || !Parse(SummaryText, Summary)) return TEXT("DuplicateManifestField");
    for (const TCHAR* Key : {TEXT("stop_reason"), TEXT("frames"), TEXT("audio")})
    {
        if (!Done.Contains(Key) || !Summary.Contains(Key) || Done[Key] != Summary[Key])
            return FString(TEXT("MissingOrMismatchedField:")) + Key;
    }
    if (Done.FindRef(TEXT("manifest")) != TEXT("written")) return TEXT("ManifestWriteFailed");
    if (Done[TEXT("stop_reason")] != TEXT("requested_stop")) return TEXT("UnexpectedRecorderStop:") + Done[TEXT("stop_reason")];
    if (Done[TEXT("frames")] != TEXT("complete")) return TEXT("IncompleteFrames:") + Done[TEXT("frames")];
    if (Done[TEXT("audio")] != TEXT("valid_pcm_nonzero")
        && !(bAllowSilentAudio && Done[TEXT("audio")] == TEXT("silent_pcm"))) return TEXT("UnverifiedAudio:") + Done[TEXT("audio")];
    auto Count = [&Summary](const TCHAR* Key, int64& Out)
    {
        const FString Value = Summary.FindRef(Key);
        if (Value.IsEmpty() || Value.Len() > 9) return false;
        for (TCHAR C : Value) if (C < TEXT('0') || C > TEXT('9')) return false;
        Out = FCString::Atoi64(*Value);
        return true;
    };
    int64 Requested = 0, Returned = 0, Saved = 0, Pending = 0;
    if (!Count(TEXT("requested"), Requested) || !Count(TEXT("returned"), Returned)
        || !Count(TEXT("saved"), Saved) || !Count(TEXT("pending_at_finalize"), Pending)
        || Requested <= 0 || Returned != Requested || Saved != Requested || Pending != 0)
        return TEXT("IncompleteFrameAccounting");
    return FString();
}

inline FString ValidateExport(const FString& Directory, bool bAllowSilentAudio = false)
{
    FString Done, Summary;
    if (!FFileHelper::LoadFileToString(Done, *(Directory / TEXT("record_done.txt")))
        || !FFileHelper::LoadFileToString(Summary, *(Directory / TEXT("complete.txt"))))
        return TEXT("MissingCompletionFiles");
    const FString Error = ValidateCompletion(Done, Summary, bAllowSilentAudio);
    if (!Error.IsEmpty()) return Error;
    if (IFileManager::Get().FileSize(*(Directory / TEXT("frames.csv"))) <= 0
        || IFileManager::Get().FileSize(*(Directory / TEXT("audio.wav"))) <= 44)
        return TEXT("MissingExportData");
    return FString();
}
}
