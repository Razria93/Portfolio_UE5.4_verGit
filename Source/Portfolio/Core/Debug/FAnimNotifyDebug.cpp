#include "Core/Debug/FAnimNotifyDebug.h"

#include "Core/Debug/FLog.h"

#include "UObject/ObjectKey.h"

namespace
{
	TSet<FObjectKey> ReportedActionNotifyTriggerWarnings;
	TSet<FObjectKey> ReportedReactionNotifyTriggerWarnings;

	bool ShouldReportNotifyWarningOnce(const UObject* InNotifyObject, TSet<FObjectKey>& InOutReportedWarnings)
	{
		if (!IsValid(InNotifyObject)) return true;

		const FObjectKey notifyKey(InNotifyObject);
		if (InOutReportedWarnings.Contains(notifyKey)) return false;

		InOutReportedWarnings.Add(notifyKey);
		return true;
	}
}

// Action Notify Warning Report

void FAnimNotifyDebug::ReportActionNotifyTriggerWarning(const UObject* InNotifyObject, const AActor* InOwnerActor, const UObject* InComponent, EActionType InTriggerActionType, int32 InTriggerActionIndex, const TCHAR* InReason)
{
	if (!ShouldReportNotifyWarningOnce(InNotifyObject, ReportedActionNotifyTriggerWarnings)) return;

	FLog::Log(FString::Printf(
		TEXT("[AnimNotify|Action|TriggerWarning] Reason=%s | Owner=%s | Component=%s | Notify=%s | TriggerActionType=%s | TriggerActionIndex=%d"),
		InReason ? InReason : TEXT("InvalidTrigger"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*GetNameSafe(InNotifyObject),
		*UEnum::GetValueAsString(InTriggerActionType),
		InTriggerActionIndex));
}

// Reaction Notify Warning Report

void FAnimNotifyDebug::ReportReactionNotifyTriggerWarning(const UObject* InNotifyObject, const AActor* InOwnerActor, const UObject* InComponent, EReactionType InTriggerReactionType, const TCHAR* InReason)
{
	if (!ShouldReportNotifyWarningOnce(InNotifyObject, ReportedReactionNotifyTriggerWarnings)) return;

	FLog::Log(FString::Printf(
		TEXT("[AnimNotify|Reaction|TriggerWarning] Reason=%s | Owner=%s | Component=%s | Notify=%s | TriggerReactionType=%s"),
		InReason ? InReason : TEXT("InvalidTrigger"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*GetNameSafe(InNotifyObject),
		*UEnum::GetValueAsString(InTriggerReactionType)));
}
